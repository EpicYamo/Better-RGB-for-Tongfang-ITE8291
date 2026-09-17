#include <windows.h>
#include <math.h>
#include <string.h>
#include "effects.h"
#include "effect_common.h"
#include "device.h"
#include "keymap.h"
#include "config.h"

DWORD WINAPI	effect_wheel(LPVOID p)
{
	double			cx;
	double			cy;
	size_t			i;
	double			t;
	double			dir_mul;
	unsigned char	buf[BUF_SIZE];
	double			x;
	double			y;
	double			angle_deg;
	double			hue;
	unsigned char	r;
	unsigned char	g;
	unsigned char	b;

	(void)p;
	cx = 0;
	cy = 0;
	i = 0;
	while (i < KEYMAP_COUNT)
	{
		cx += get_key_x(&KEYMAP[i]);
		cy += KEYMAP[i].row;
		i++;
	}
	cx /= KEYMAP_COUNT;
	cy /= KEYMAP_COUNT;
	t = 0.0;
	while (!g_stopFlag)
	{
		if (g_wheelReverse)
			dir_mul = -1.0;
		else
			dir_mul = 1.0;
		memset(buf, 0, BUF_SIZE);
		i = 0;
		while (i < KEYMAP_COUNT)
		{
			x = get_key_x(&KEYMAP[i]) - cx;
			y = (double)KEYMAP[i].row - cy;
			angle_deg = atan2(y, x) * 180.0 / M_PI;
			hue = t * dir_mul + angle_deg;
			hsv_to_rgb(hue, &r, &g, &b);
			set_key(buf, KEYMAP[i].offset, r, g, b);
			i++;
		}
		send_frame(buf);
		t += (double)g_wheelSpeed;
		Sleep(FRAME_DELAY_MS);
	}
	return (0);
}
