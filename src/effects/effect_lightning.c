#include <windows.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "effects.h"
#include "effect_common.h"
#include "device.h"
#include "keymap.h"
#include "config.h"

#define MAX_STRIKES	4

DWORD WINAPI	effect_lightning(LPVOID p)
{
	double			head_pos[MAX_STRIKES];
	double			bolt_x[MAX_STRIKES];
	int				countdown[MAX_STRIKES];
	int				i;
	size_t			ui;
	int				s;
	double			speed;
	double			band_width;
	double			tail_len;
	double			finish_pos;
	double			x_width;
	int				concurrent;
	double			bright[KEYMAP_COUNT];
	double			dv;
	double			dh;
	double			v_fall;
	double			h_fall;
	double			b;
	unsigned char	buf[BUF_SIZE];
	unsigned char	v;

	(void)p;
	i = 0;
	while (i < MAX_STRIKES)
	{
		head_pos[i] = 100.0;
		bolt_x[i] = 10.0;
		countdown[i] = 10 + rand() % 30;
		i++;
	}
	while (!g_stopFlag)
	{
		speed = 0.05 + (double)g_lightningSpeed * 0.08;
		band_width = 0.4 + (double)g_lightningSmooth * 0.15;
		tail_len = band_width * 2.5;
		finish_pos = 6.0 + tail_len + 0.5;
		x_width = 0.35 + ((double)g_lightningWidth - 1.0) * 0.383;
		concurrent = (int)g_lightningConcurrent;
		if (concurrent < 1)
			concurrent = 1;
		if (concurrent > MAX_STRIKES)
			concurrent = MAX_STRIKES;
		ui = 0;
		while (ui < KEYMAP_COUNT)
		{
			bright[ui] = 0;
			ui++;
		}
		s = 0;
		while (s < concurrent)
		{
			if (head_pos[s] > finish_pos)
			{
				countdown[s]--;
				if (countdown[s] <= 0)
				{
					head_pos[s] = -2.0;
					bolt_x[s] = (double)(rand() % 20);
					countdown[s] = 20 + rand() % 40;
				}
			}
			if (head_pos[s] <= finish_pos)
			{
				ui = 0;
				while (ui < KEYMAP_COUNT)
				{
					dv = head_pos[s] - KEYMAP[ui].row;
					dh = fabs(get_key_x(&KEYMAP[ui]) - bolt_x[s]);
					if (dv < -0.3 || dv > tail_len)
					{
						ui++;
						continue;
					}
					if (dh > x_width)
					{
						ui++;
						continue;
					}
					if (dv < 0)
						v_fall = 1.0;
					else
						v_fall = 1.0 - dv / tail_len;
					if (v_fall < 0)
						v_fall = 0;
					h_fall = 1.0 - dh / x_width;
					if (h_fall < 0)
						h_fall = 0;
					b = v_fall * h_fall;
					if (b > bright[ui])
						bright[ui] = b;
					ui++;
				}
				head_pos[s] += speed;
			}
			s++;
		}
		memset(buf, 0, BUF_SIZE);
		ui = 0;
		while (ui < KEYMAP_COUNT)
		{
			if (bright[ui] > 0)
			{
				v = (unsigned char)(bright[ui] * 255);
				set_key(buf, KEYMAP[ui].offset, v, v, 255);
			}
			else
				set_key(buf, KEYMAP[ui].offset, 4, 4, 14);
			ui++;
		}
		send_frame(buf);
		Sleep(FRAME_DELAY_MS);
	}
	return (0);
}
