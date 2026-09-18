#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "effects.h"
#include "effect_common.h"
#include "device.h"
#include "config.h"
#include "log.h"

volatile LONG	g_brightness = 100;

static LPTHREAD_START_ROUTINE	mode_to_fn(int mode)
{
	if (mode == MODE_BREATH)
		return (effect_breathing);
	if (mode == MODE_WAVE)
		return (effect_wave);
	if (mode == MODE_SPARKLE)
		return (effect_sparkle);
	if (mode == MODE_REACTIVE)
		return (effect_reactive);
	if (mode == MODE_WHEEL)
		return (effect_wheel);
	if (mode == MODE_LIGHTNING)
		return (effect_lightning);
	if (mode == MODE_FLAME)
		return (effect_flame);
	if (mode == MODE_RAIN)
		return (effect_rain);
	if (mode == MODE_MATRIX)
		return (effect_matrix);
	if (mode == MODE_STATIC)
		return (effect_static);
	return (NULL);
}

static const wchar_t	*mode_to_label(int mode)
{
	if (mode == MODE_BREATH)
		return (L"Active Mode: Breathing");
	if (mode == MODE_WAVE)
		return (L"Active Mode: Wave");
	if (mode == MODE_SPARKLE)
		return (L"Active Mode: Sparkle");
	if (mode == MODE_REACTIVE)
		return (L"Active Mode: Reactive");
	if (mode == MODE_WHEEL)
		return (L"Active Mode: Wheel");
	if (mode == MODE_LIGHTNING)
		return (L"Active Mode: Lightning");
	if (mode == MODE_FLAME)
		return (L"Active Mode: Flame");
	if (mode == MODE_RAIN)
		return (L"Active Mode: Rain");
	if (mode == MODE_MATRIX)
		return (L"Active Mode: Matrix");
	if (mode == MODE_STATIC)
		return (L"Active Mode: Static");
	return (L"Active Mode: None");
}

void	set_status(const wchar_t *text)
{
	if (g_hStatus)
		SetWindowText(g_hStatus, text);
}

void	stop_current_effect(void)
{
	unsigned char	blank[BUF_SIZE];

	if (g_effectThread)
	{
		log_event("stop_current_effect: stopping active effect thread, sending blank frame");
		InterlockedExchange(&g_stopFlag, 1);
		WaitForSingleObject(g_effectThread, 2000);
		CloseHandle(g_effectThread);
		g_effectThread = NULL;
		memset(blank, 0, BUF_SIZE);
		send_frame(blank);
	}
	g_activeMode = -1;
}

void	start_mode(int mode)
{
	char	logbuf[96];
	int		reinit;

	sprintf(logbuf, "start_mode: switching to mode %d", mode);
	log_event(logbuf);
	stop_current_effect();
	EnterCriticalSection(&g_deviceLock);
	if (g_h)
		reinit = enter_custom_mode();
	else
		reinit = -1;
	LeaveCriticalSection(&g_deviceLock);
	if (reinit < 0)
		sprintf(logbuf, "start_mode: full EC re-init (prep1+prep2) result=FAIL");
	else
		sprintf(logbuf, "start_mode: full EC re-init (prep1+prep2) result=ok");
	log_event(logbuf);
	if (reinit < 0)
		request_reconnect("start_mode-reinit-failed");
	InterlockedExchange(&g_stopFlag, 0);
	g_activeMode = mode;
	g_effectThread = CreateThread(NULL, 0, mode_to_fn(mode), NULL, 0, NULL);
	if (g_effectThread)
		SetThreadPriority(g_effectThread, THREAD_PRIORITY_TIME_CRITICAL);
	set_status(mode_to_label(mode));
	save_config();
}
