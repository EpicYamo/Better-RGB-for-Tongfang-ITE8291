#include <windows.h>
#include <math.h>
#include <string.h>
#include "effects.h"
#include "effect_common.h"
#include "device.h"
#include "keymap.h"
#include "config.h"

DWORD WINAPI	effect_breathing(LPVOID p)
{
	double			t;
	unsigned char	br;
	unsigned char	bg;
	unsigned char	bb;
	double			raw;
	int				levels;
	double			bright;
	unsigned char	r;
	unsigned char	g;
	unsigned char	b;
	unsigned char	buf[BUF_SIZE];
	size_t			i;

	(void)p;
	t = 0.0;
	while (!g_stopFlag)
	{
		br = GetRValue(g_breathColor);
		bg = GetGValue(g_breathColor);
		bb = GetBValue(g_breathColor);
		raw = (sin(t) + 1.0) / 2.0;
		levels = (int)g_breathSmooth;
		if (levels < 2)
			levels = 2;
		bright = floor(raw * levels) / (double)levels;
		r = (unsigned char)(br * bright);
		g = (unsigned char)(bg * bright);
		b = (unsigned char)(bb * bright);
		memset(buf, 0, BUF_SIZE);
		i = 0;
		while (i < KEYMAP_COUNT)
		{
			set_key(buf, KEYMAP[i].offset, r, g, b);
			i++;
		}
		send_frame(buf);
		t += (double)g_breathSpeed / 100.0;
		Sleep(FRAME_DELAY_MS);
	}
	return (0);
}
