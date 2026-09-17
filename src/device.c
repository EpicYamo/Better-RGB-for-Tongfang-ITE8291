#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "hidapi.h"
#include "device.h"
#include "log.h"

hid_device			*g_h = NULL;
CRITICAL_SECTION	g_deviceLock;
char				g_devicePath[512] = "";
volatile LONG		g_brightness = 100;

const GUID	GUID_DEVINTERFACE_HID_LOCAL =
	{0x4D1E55B2, 0xF16F, 0x11CF, {0x88, 0xCB, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30}};

static volatile LONG	g_reconnectInProgress = 0;

int	resolve_device_path(void)
{
	char					lb[640];
	struct hid_device_info	*list;
	struct hid_device_info	*d;
	struct hid_device_info	*best;
	int						best_score;
	int						score;
	int						found;

	list = hid_enumerate(0x048D, 0x0000);
	if (!list)
	{
		log_event("enum: no ITE (VID_048D) HID devices found on this system");
		return (0);
	}
	best = NULL;
	best_score = 0;
	d = list;
	while (d)
	{
		sprintf(lb, "enum: found VID=%04X PID=%04X usage_page=0x%04X usage=0x%04X iface=%d path=%.400s",
			d->vendor_id, d->product_id, d->usage_page, d->usage,
			d->interface_number, d->path ? d->path : "(null)");
		log_event(lb);
		score = 0;
		if (d->usage_page == 0xFF03)
			score = 3;
		else if (d->interface_number == 1)
			score = 2;
		if (score && d->product_id == 0x600B)
			score++;
		if (score > best_score)
		{
			best_score = score;
			best = d;
		}
		d = d->next;
	}
	found = 0;
	if (best && best->path)
	{
		strncpy(g_devicePath, best->path, sizeof(g_devicePath) - 1);
		g_devicePath[sizeof(g_devicePath) - 1] = 0;
		sprintf(lb, "enum: selected VID=%04X PID=%04X usage_page=0x%04X iface=%d",
			best->vendor_id, best->product_id, best->usage_page, best->interface_number);
		log_event(lb);
		found = 1;
	}
	else
		log_event("enum: ITE device present but no vendor RGB interface (usage page 0xFF03 / MI_01) found");
	hid_free_enumeration(list);
	return (found);
}

int	enter_custom_mode(void)
{
	unsigned char	step1[9] = {0x00, 0x12, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00};
	unsigned char	clear_frame[65] = {0};
	unsigned char	step2[9] = {0x00, 0x08, 0x02, 0x33, 0x00, 0x32, 0x00, 0x00, 0x00};

	if (hid_send_feature_report(g_h, step1, 9) < 0)
		return (-1);
	hid_write(g_h, clear_frame, 65);
	if (hid_send_feature_report(g_h, step2, 9) < 0)
		return (-1);
	return (0);
}

int	log_exclusive_status(void)
{
	HANDLE	probe;

	probe = CreateFileA(g_devicePath, GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
			FILE_FLAG_OVERLAPPED, 0);
	if (probe == INVALID_HANDLE_VALUE && GetLastError() == ERROR_SHARING_VIOLATION)
	{
		log_event("device lock: EXCLUSIVE - no other process can open the RGB interface");
		return (1);
	}
	if (probe != INVALID_HANDLE_VALUE)
		CloseHandle(probe);
	log_event("device lock: SHARED - exclusive open not in effect; GCUBridge takeovers remain possible");
	return (0);
}

int	reconnect_device(void)
{
	EnterCriticalSection(&g_deviceLock);
	log_event("reconnect_device: starting");
	if (g_h)
	{
		hid_close(g_h);
		g_h = NULL;
	}
	hid_exit();
	Sleep(100);
	hid_init();
	Sleep(20);
	if (!resolve_device_path())
	{
		log_event("reconnect_device: device enumeration FAILED");
		LeaveCriticalSection(&g_deviceLock);
		return (-1);
	}
	g_h = hid_open_path(g_devicePath);
	if (!g_h)
	{
		log_event("reconnect_device: hid_open_path FAILED");
		LeaveCriticalSection(&g_deviceLock);
		return (-1);
	}
	Sleep(20);
	if (enter_custom_mode() < 0)
	{
		log_event("reconnect_device: enter_custom_mode FAILED");
		hid_close(g_h);
		g_h = NULL;
		LeaveCriticalSection(&g_deviceLock);
		return (-1);
	}
	log_event("reconnect_device: success");
	log_exclusive_status();
	LeaveCriticalSection(&g_deviceLock);
	return (0);
}

static DWORD WINAPI	reconnect_thread_proc(LPVOID p)
{
	(void)p;
	reconnect_device();
	InterlockedExchange(&g_reconnectInProgress, 0);
	return (0);
}

void	request_reconnect(const char *reason)
{
	char	logbuf[160];
	HANDLE	h;

	if (InterlockedCompareExchange(&g_reconnectInProgress, 1, 0) != 0)
	{
		sprintf(logbuf, "request_reconnect(%s): skipped, one already in progress", reason);
		log_event(logbuf);
		return ;
	}
	sprintf(logbuf, "request_reconnect(%s): spawning reconnect thread", reason);
	log_event(logbuf);
	h = CreateThread(NULL, 0, reconnect_thread_proc, NULL, 0, NULL);
	if (h)
		CloseHandle(h);
}

int	send_frame(unsigned char *buf)
{
	ULONGLONG		t_start;
	ULONGLONG		t_locked;
	ULONGLONG		t_done;
	ULONGLONG		lock_wait;
	ULONGLONG		call_time;
	unsigned char	scaled[BUF_SIZE];
	unsigned char	chunk[65];
	unsigned char	step3[9] = {0x00, 0x12, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00};
	char			logbuf[160];
	int				bright;
	size_t			i;

	t_start = GetTickCount64();
	EnterCriticalSection(&g_deviceLock);
	t_locked = GetTickCount64();
	bright = (int)g_brightness;
	if (bright < 100)
	{
		i = 0;
		while (i < BUF_SIZE)
		{
			scaled[i] = (unsigned char)((int)buf[i] * bright / 100);
			i++;
		}
		buf = scaled;
	}
	if (hid_send_feature_report(g_h, step3, 9) < 0)
	{
		log_event("send_frame: feature report failed, attempting reconnect");
		if (reconnect_device() < 0)
		{
			LeaveCriticalSection(&g_deviceLock);
			return (-1);
		}
		if (hid_send_feature_report(g_h, step3, 9) < 0)
		{
			LeaveCriticalSection(&g_deviceLock);
			return (-1);
		}
	}
	i = 0;
	while (i < 8)
	{
		chunk[0] = 0x00;
		memcpy(chunk + 1, buf + i * 64, 64);
		if (hid_write(g_h, chunk, 65) < 0)
		{
			log_event("send_frame: chunk write failed, attempting reconnect");
			if (reconnect_device() < 0)
			{
				LeaveCriticalSection(&g_deviceLock);
				return (-1);
			}
			LeaveCriticalSection(&g_deviceLock);
			return (-1);
		}
		i++;
	}
	t_done = GetTickCount64();
	LeaveCriticalSection(&g_deviceLock);
	lock_wait = t_locked - t_start;
	call_time = t_done - t_locked;
	if (lock_wait > 200 || call_time > 200)
	{
		sprintf(logbuf, "send_frame SLOW: lock_wait=%llums call_time=%llums",
			(unsigned long long)lock_wait, (unsigned long long)call_time);
		log_event(logbuf);
	}
	return (0);
}

void	set_key(unsigned char *buf, int offset, unsigned char r, unsigned char g, unsigned char b)
{
	if (offset < 0 || offset + 2 >= BUF_SIZE)
		return ;
	buf[offset] = r;
	buf[offset + 1] = g;
	buf[offset + 2] = b;
}
