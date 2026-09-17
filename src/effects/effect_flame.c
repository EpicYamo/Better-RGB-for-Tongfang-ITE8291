#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include "effects.h"
#include "effect_common.h"
#include "device.h"
#include "keymap.h"
#include "config.h"

DWORD WINAPI	effect_flame(LPVOID p)
{
	double			heat[KEYMAP_COUNT];
	size_t			i;
	int				frame_counter;
	double			speed;
	double			smooth;
	double			alpha;
	double			volatility;
	int				update_interval;
	double			row_frac;
	double			bias;
	double			target;
	unsigned char	buf[BUF_SIZE];
	int				h;
	unsigned char	r;
	unsigned char	g;
	unsigned char	b;

	(void)p;
	i = 0;
	while (i < KEYMAP_COUNT)
	{
		heat[i] = 100.0 + rand() % 60;
		i++;
	}
	frame_counter = 0;
	while (!g_stopFlag)
	{
		speed = (double)g_flameSpeed;
		smooth = (double)g_flameSmooth;
		alpha = 1.0 / (1.0 + smooth * 0.6);
		volatility = 40.0 + speed * 18.0;
		update_interval = 11 - (int)speed;
		if (update_interval < 1)
			update_interval = 1;
		frame_counter++;
		if (frame_counter % update_interval == 0)
		{
			i = 0;
			while (i < KEYMAP_COUNT)
			{
				row_frac = KEYMAP[i].row / 6.0;
				bias = 40.0 * row_frac;
				target = 90.0 + bias + (rand() % (int)volatility - volatility / 2.0);
				heat[i] = heat[i] * (1.0 - alpha) + target * alpha;
				if (heat[i] < 0)
					heat[i] = 0;
				if (heat[i] > 255)
					heat[i] = 255;
				i++;
			}
		}
		memset(buf, 0, BUF_SIZE);
		i = 0;
		while (i < KEYMAP_COUNT)
		{
			h = (int)heat[i];
			if (h < 85)
			{
				r = (unsigned char)(h * 3);
				g = 0;
				b = 0;
			}
			else if (h < 170)
			{
				r = 255;
				g = (unsigned char)((h - 85) * 3);
				b = 0;
			}
			else
			{
				r = 255;
				g = 255;
				b = (unsigned char)((h - 170) * 3);
			}
			set_key(buf, KEYMAP[i].offset, r, g, b);
			i++;
		}
		send_frame(buf);
		Sleep(FRAME_DELAY_MS);
	}
	return (0);
}
