#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "log.h"

static char	g_logPath[MAX_PATH] = "";

void	build_sibling_path(char *dst, const char *name)
{
	char	*p;

	GetModuleFileNameA(NULL, dst, MAX_PATH);
	p = strrchr(dst, '\\');
	if (p)
		strcpy(p + 1, name);
	else
		strcpy(dst, name);
}

void	log_event(const char *msg)
{
	FILE		*f;
	SYSTEMTIME	t;

	if (!g_logPath[0])
		build_sibling_path(g_logPath, "rgb_engine_log.txt");
	f = fopen(g_logPath, "a");
	if (!f)
		return ;
	GetLocalTime(&t);
	fprintf(f, "[%02d:%02d:%02d.%03d] [thread %lu] %s\n",
		t.wHour, t.wMinute, t.wSecond, t.wMilliseconds,
		(unsigned long)GetCurrentThreadId(), msg);
	fclose(f);
}
