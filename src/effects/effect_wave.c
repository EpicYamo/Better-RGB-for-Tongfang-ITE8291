#include <windows.h>
#include <math.h>
#include <string.h>
#include "effects.h"
#include "effect_common.h"
#include "device.h"
#include "keymap.h"
#include "config.h"

DWORD WINAPI	effect_wave(LPVOID p)
{
	double			t;
	double			angle_rad;
	double			dir_x;
	double			dir_y;
	unsigned char	buf[BUF_SIZE];
	size_t			i;
	double			x;
	double			y;
	double			proj;
	double			hue;
	unsigned char	r;
	unsigned char	g;
	unsigned char	b;

	(void)p;
	t = 0.0;
	while (!g_stopFlag)
	{
		angle_rad = (double)g_waveAngle * M_PI / 180.0;
		dir_x = cos(angle_rad);
		dir_y = sin(angle_rad);
		memset(buf, 0, BUF_SIZE);
		i = 0;
		while (i < KEYMAP_COUNT)
		{
			x = get_key_x(&KEYMAP[i]);
			y = (double)KEYMAP[i].row;
			proj = x * dir_x + y * dir_y;
			hue = t + proj * 15.0;
			hsv_to_rgb(hue, &r, &g, &b);
			set_key(buf, KEYMAP[i].offset, r, g, b);
			i++;
		}
		send_frame(buf);
		t += (double)g_waveSpeed;
		Sleep(FRAME_DELAY_MS);
	}
	return (0);
}
