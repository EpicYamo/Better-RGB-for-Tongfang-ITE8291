#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "hidapi.h"
#include "vendor_lock.h"
#include "device.h"
#include "log.h"

# define VENDOR_SERVICE_DEFAULT	"GCUBridge"

int	is_elevated(void)
{
	HANDLE			tok;
	TOKEN_ELEVATION	te;
	DWORD			n;
	int				r;

	r = 0;
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &tok))
	{
		if (GetTokenInformation(tok, TokenElevation, &te, sizeof(te), &n))
		{
			if (te.TokenIsElevated)
				r = 1;
			else
				r = 0;
		}
		CloseHandle(tok);
	}
	return (r);
}

static int	find_vendor_service(char *out, DWORD out_len)
{
	SC_HANDLE						scm;
	SC_HANDLE						svc;
	int								found;
	DWORD							bytes_needed;
	DWORD							count;
	DWORD							resume;
	BYTE							*buf;
	ENUM_SERVICE_STATUS_PROCESSA	*e;
	DWORD							i;
	char							low[256];
	char							*p;

	scm = OpenSCManagerA(NULL, NULL, SC_MANAGER_CONNECT | SC_MANAGER_ENUMERATE_SERVICE);
	if (!scm)
		return (0);
	svc = OpenServiceA(scm, VENDOR_SERVICE_DEFAULT, SERVICE_QUERY_STATUS);
	if (svc)
	{
		CloseServiceHandle(svc);
		CloseServiceHandle(scm);
		strncpy(out, VENDOR_SERVICE_DEFAULT, out_len - 1);
		out[out_len - 1] = 0;
		return (1);
	}
	found = 0;
	bytes_needed = 0;
	count = 0;
	resume = 0;
	EnumServicesStatusExA(scm, SC_ENUM_PROCESS_INFO, SERVICE_WIN32, SERVICE_STATE_ALL,
		NULL, 0, &bytes_needed, &count, &resume, NULL);
	if (bytes_needed > 0)
	{
		buf = (BYTE *)malloc(bytes_needed);
		if (buf && EnumServicesStatusExA(scm, SC_ENUM_PROCESS_INFO, SERVICE_WIN32, SERVICE_STATE_ALL,
				buf, bytes_needed, &bytes_needed, &count, &resume, NULL))
		{
			e = (ENUM_SERVICE_STATUS_PROCESSA *)buf;
			i = 0;
			while (i < count && !found)
			{
				strncpy(low, e[i].lpServiceName, sizeof(low) - 1);
				low[sizeof(low) - 1] = 0;
				p = low;
				while (*p)
				{
					*p = (char)tolower((unsigned char)*p);
					p++;
				}
				if (strstr(low, "gcu"))
				{
					strncpy(out, e[i].lpServiceName, out_len - 1);
					out[out_len - 1] = 0;
					found = 1;
				}
				i++;
			}
		}
		if (buf)
			free(buf);
	}
	CloseServiceHandle(scm);
	return (found);
}

static int	stop_vendor_service(const char *name)
{
	SC_HANDLE		scm;
	SC_HANDLE		svc;
	SERVICE_STATUS	ss;
	int				i;
	int				ok;

	scm = OpenSCManagerA(NULL, NULL, SC_MANAGER_CONNECT);
	if (!scm)
		return (-1);
	svc = OpenServiceA(scm, name, SERVICE_STOP | SERVICE_QUERY_STATUS);
	if (!svc)
	{
		CloseServiceHandle(scm);
		return (-1);
	}
	memset(&ss, 0, sizeof(ss));
	QueryServiceStatus(svc, &ss);
	if (ss.dwCurrentState != SERVICE_STOPPED)
	{
		ControlService(svc, SERVICE_CONTROL_STOP, &ss);
		i = 0;
		while (i < 100)
		{
			if (!QueryServiceStatus(svc, &ss))
				break ;
			if (ss.dwCurrentState == SERVICE_STOPPED)
				break ;
			Sleep(100);
			i++;
		}
	}
	if (ss.dwCurrentState == SERVICE_STOPPED)
		ok = 0;
	else
		ok = -1;
	CloseServiceHandle(svc);
	CloseServiceHandle(scm);
	return (ok);
}

static int	start_vendor_service(const char *name)
{
	SC_HANDLE	scm;
	SC_HANDLE	svc;
	int			ok;

	scm = OpenSCManagerA(NULL, NULL, SC_MANAGER_CONNECT);
	if (!scm)
		return (-1);
	svc = OpenServiceA(scm, name, SERVICE_START);
	if (!svc)
	{
		CloseServiceHandle(scm);
		return (-1);
	}
	ok = 0;
	if (!StartServiceA(svc, 0, NULL) && GetLastError() != ERROR_SERVICE_ALREADY_RUNNING)
		ok = -1;
	CloseServiceHandle(svc);
	CloseServiceHandle(scm);
	return (ok);
}

void	restart_vendor_service_if_found(void)
{
	char	svc_name[256];
	char	lb[384];

	if (find_vendor_service(svc_name, sizeof(svc_name)))
	{
		start_vendor_service(svc_name);
		sprintf(lb, "relock: vendor service '%s' restarted", svc_name);
		log_event(lb);
	}
	else
		log_event("relock: vendor service not found for restart");
}

int	begin_relock(void)
{
	char				svc_name[256];
	char				lb[384];
	wchar_t				exe[MAX_PATH];
	wchar_t				cmd_line[MAX_PATH + 32];
	STARTUPINFOW		si;
	PROCESS_INFORMATION	pi;

	if (!find_vendor_service(svc_name, sizeof(svc_name)))
	{
		log_event("relock: vendor service not found, cannot fix automatically");
		return (-1);
	}
	sprintf(lb, "relock: stopping vendor service '%s'", svc_name);
	log_event(lb);
	if (stop_vendor_service(svc_name) < 0)
	{
		log_event("relock: could not stop vendor service");
		return (-1);
	}
	EnterCriticalSection(&g_deviceLock);
	if (g_h)
	{
		hid_close(g_h);
		g_h = NULL;
	}
	LeaveCriticalSection(&g_deviceLock);
	GetModuleFileNameW(NULL, exe, MAX_PATH);
	if (g_startupLaunch)
		_snwprintf(cmd_line, MAX_PATH + 31, L"\"%ls\" --relock %lu --startup",
			exe, (unsigned long)GetCurrentProcessId());
	else
		_snwprintf(cmd_line, MAX_PATH + 31, L"\"%ls\" --relock %lu",
			exe, (unsigned long)GetCurrentProcessId());
	cmd_line[MAX_PATH + 31] = 0;
	memset(&si, 0, sizeof(si));
	memset(&pi, 0, sizeof(pi));
	si.cb = sizeof(si);
	if (!CreateProcessW(NULL, cmd_line, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
	{
		log_event("relock: failed to spawn fresh process, restoring previous state");
		start_vendor_service(svc_name);
		EnterCriticalSection(&g_deviceLock);
		g_h = hid_open_path(g_devicePath);
		if (g_h)
			enter_custom_mode();
		LeaveCriticalSection(&g_deviceLock);
		return (-1);
	}
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	log_event("relock: fresh process spawned, this instance is exiting");
	return (0);
}

int	relaunch_elevated(const wchar_t *args, int wait_for_exit)
{
	wchar_t				exe[MAX_PATH];
	SHELLEXECUTEINFOW	sei;

	GetModuleFileNameW(NULL, exe, MAX_PATH);
	memset(&sei, 0, sizeof(sei));
	sei.cbSize = sizeof(sei);
	sei.fMask = SEE_MASK_NOCLOSEPROCESS;
	sei.lpVerb = L"runas";
	sei.lpFile = exe;
	sei.lpParameters = args;
	sei.nShow = SW_SHOWNORMAL;
	if (!ShellExecuteExW(&sei))
		return (-1);
	if (wait_for_exit && sei.hProcess)
		WaitForSingleObject(sei.hProcess, 120000);
	if (sei.hProcess)
		CloseHandle(sei.hProcess);
	return (0);
}
