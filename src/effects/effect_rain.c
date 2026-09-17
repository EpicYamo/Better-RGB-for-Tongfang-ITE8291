#include <windows.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "effects.h"
#include "effect_common.h"
#include "device.h"
#include "keymap.h"
#include "config.h"

#define MAX_RAIN_DROPS	20
#define RAIN_DROPS		MAX_RAIN_DROPS

DWORD WINAPI	effect_rain(LPVOID p)
{
	double			drop_x[RAIN_DROPS];
	double			drop_row[RAIN_DROPS];
	int				drop_color_idx[RAIN_DROPS];
	int				i;
	int				active_drops;
	double			speed;
	int				color_count;
	int				target;
	unsigned char	buf[BUF_SIZE];
	int				d;
	COLORREF		col;
	unsigned char	cr;
	unsigned char	cg;
	unsigned char	cb;
	int				trail_row;
	double			trail_dist;
	double			bright;
	int				best;
	double			best_dist;
	size_t			ki;
	double			dx;

	(void)p;
	i = 0;
	while (i < RAIN_DROPS)
	{
		drop_x[i] = rand() % 20;
		drop_row[i] = -(double)(rand() % 8);
		drop_color_idx[i] = rand() % (int)g_rainColorCount;
		i++;
	}
	active_drops = (int)g_rainDensityTarget * 3;
	while (!g_stopFlag)
	{
		speed = 0.05 + (double)g_rainSpeed * 0.03;
		color_count = (int)g_rainColorCount;
		if (color_count < 1)
			color_count = 1;
		if (color_count > 5)
			color_count = 5;
		target = (int)g_rainDensityTarget;
		if (target < 1)
			target = 1;
		if (target > 5)
			target = 5;
		active_drops = target * 3;
		if (active_drops > MAX_RAIN_DROPS)
			active_drops = MAX_RAIN_DROPS;
		memset(buf, 0, BUF_SIZE);
		d = 0;
		while (d < active_drops)
		{
			col = g_rainColors[drop_color_idx[d] % color_count];
			cr = GetRValue(col);
			cg = GetGValue(col);
			cb = GetBValue(col);
			trail_row = 0;
			while (trail_row <= 6)
			{
				trail_dist = drop_row[d] - trail_row;
				if (trail_dist < -0.3 || trail_dist > 1.8)
				{
					trail_row++;
					continue;
				}
				if (trail_dist < 0)
					bright = 1.0;
				else
					bright = 1.0 - trail_dist / 1.8;
				if (bright < 0)
					bright = 0;
				best = -1;
				best_dist = 1e9;
				ki = 0;
				while (ki < KEYMAP_COUNT)
				{
					if (KEYMAP[ki].row == trail_row)
					{
						dx = fabs(get_key_x(&KEYMAP[ki]) - drop_x[d]);
						if (dx < best_dist)
						{
							best_dist = dx;
							best = (int)ki;
						}
					}
					ki++;
				}
				if (best >= 0 && best_dist < 1.2)
				{
					set_key(buf, KEYMAP[best].offset, (unsigned char)(cr * bright),
						(unsigned char)(cg * bright), (unsigned char)(cb * bright));
				}
				trail_row++;
			}
			drop_row[d] += speed;
			if (drop_row[d] > 8)
			{
				drop_row[d] = -(double)(rand() % 5);
				drop_x[d] = rand() % 20;
				drop_color_idx[d] = rand() % color_count;
			}
			d++;
		}
		send_frame(buf);
		Sleep(FRAME_DELAY_MS);
	}
	return (0);
}
