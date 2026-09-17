#include <windows.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "effects.h"
#include "effect_common.h"
#include "device.h"
#include "keymap.h"
#include "config.h"

#define MATRIX_STREAKS	5
#define MATRIX_COLUMNS	24
#define MAX_LASERS		3
#define MAX_RIPPLES		3

DWORD WINAPI	effect_matrix(LPVOID p)
{
	double			angle[MATRIX_STREAKS];
	double			pos[MATRIX_STREAKS];
	double			spd[MATRIX_STREAKS];
	int				streak_color_idx[MATRIX_STREAKS];
	int				active_streaks;
	int				reroll_counter;
	double			col_pos[MATRIX_COLUMNS];
	double			col_speed[MATRIX_COLUMNS];
	int				col_color_idx[MATRIX_COLUMNS];
	double			laser_angle[MAX_LASERS];
	double			laser_pos[MAX_LASERS];
	double			laser_spd[MAX_LASERS];
	int				laser_color_idx[MAX_LASERS];
	double			ripple_x[MAX_RIPPLES];
	double			ripple_y[MAX_RIPPLES];
	double			ripple_r[MAX_RIPPLES];
	int				ripple_color_idx[MAX_RIPPLES];
	int				i;
	size_t			ki;
	int				color_count;
	int				style;
	double			speed_mul;
	unsigned char	buf[BUF_SIZE];
	double			x;
	double			y;
	double			best_bright;
	int				best_color;
	int				s;
	double			dir_x;
	double			dir_y;
	double			proj;
	double			dist;
	double			b;
	COLORREF		col;
	unsigned char	cr;
	unsigned char	cg;
	unsigned char	cb;
	int				column;
	double			dv;
	COLORREF		cc;
	int				target;
	int				laser_count;
	double			dx;
	double			dy;
	double			ring_dist;

	(void)p;
	i = 0;
	while (i < MATRIX_STREAKS)
	{
		angle[i] = (rand() % 360) * M_PI / 180.0;
		pos[i] = -30.0;
		spd[i] = 0.2 + (rand() % 100) / 100.0;
		streak_color_idx[i] = rand() % (int)g_matrixColorCount;
		i++;
	}
	active_streaks = (int)g_matrixDensityTarget;
	reroll_counter = 0;
	i = 0;
	while (i < MATRIX_COLUMNS)
	{
		col_pos[i] = -(double)(rand() % 8);
		col_speed[i] = 0.05 + (rand() % 100) / 300.0;
		col_color_idx[i] = rand() % (int)g_matrixColorCount;
		i++;
	}
	i = 0;
	while (i < MAX_LASERS)
	{
		laser_angle[i] = (rand() % 4) * (M_PI / 2.0);
		laser_pos[i] = -30.0 - i * 8.0;
		laser_spd[i] = 0.6 + (rand() % 100) / 100.0;
		laser_color_idx[i] = rand() % (int)g_matrixColorCount;
		i++;
	}
	i = 0;
	while (i < MAX_RIPPLES)
	{
		ripple_x[i] = rand() % 20;
		ripple_y[i] = rand() % 7;
		ripple_r[i] = i * 8.0;
		ripple_color_idx[i] = rand() % (int)g_matrixColorCount;
		i++;
	}
	while (!g_stopFlag)
	{
		color_count = (int)g_matrixColorCount;
		if (color_count < 1)
			color_count = 1;
		if (color_count > 3)
			color_count = 3;
		style = (int)g_matrixStyle;
		speed_mul = 0.3 + (double)g_matrixSpeed * 0.15;
		memset(buf, 0, BUF_SIZE);
		if (style == 0)
		{
			reroll_counter++;
			if (reroll_counter >= 60)
			{
				active_streaks = compute_prob_count((int)g_matrixDensityTarget, 1, 5);
				reroll_counter = 0;
			}
			ki = 0;
			while (ki < KEYMAP_COUNT)
			{
				x = get_key_x(&KEYMAP[ki]);
				y = KEYMAP[ki].row;
				best_bright = 0;
				best_color = 0;
				s = 0;
				while (s < active_streaks)
				{
					dir_x = cos(angle[s]);
					dir_y = sin(angle[s]);
					proj = x * dir_x + y * dir_y;
					dist = fabs(proj - pos[s]);
					if (dist < 1.5)
					{
						b = 1.0 - dist / 1.5;
						if (b > best_bright)
						{
							best_bright = b;
							best_color = streak_color_idx[s] % color_count;
						}
					}
					s++;
				}
				if (best_bright > 1)
					best_bright = 1;
				col = g_matrixColors[best_color];
				cr = (unsigned char)(GetRValue(col) * best_bright);
				cg = (unsigned char)(GetGValue(col) * best_bright);
				cb = (unsigned char)(GetBValue(col) * best_bright);
				set_key(buf, KEYMAP[ki].offset, cr, cg, cb);
				ki++;
			}
			s = 0;
			while (s < active_streaks)
			{
				pos[s] += spd[s] * speed_mul;
				if (pos[s] > 30)
				{
					pos[s] = -30;
					angle[s] = (rand() % 360) * M_PI / 180.0;
					spd[s] = 0.2 + (rand() % 100) / 100.0;
					streak_color_idx[s] = rand() % color_count;
				}
				s++;
			}
		}
		else if (style == 1)
		{
			ki = 0;
			while (ki < KEYMAP_COUNT)
			{
				x = get_key_x(&KEYMAP[ki]);
				column = (int)(x + 0.5);
				if (column < 0)
					column = 0;
				if (column >= MATRIX_COLUMNS)
					column = MATRIX_COLUMNS - 1;
				dv = col_pos[column] - KEYMAP[ki].row;
				best_bright = 0;
				if (dv >= -0.3 && dv < 4.0)
				{
					if (dv < 0)
						best_bright = 1.0;
					else
						best_bright = 1.0 - dv / 4.0;
				}
				if (best_bright < 0)
					best_bright = 0;
				cc = g_matrixColors[col_color_idx[column] % color_count];
				cr = (unsigned char)(GetRValue(cc) * best_bright);
				cg = (unsigned char)(GetGValue(cc) * best_bright);
				cb = (unsigned char)(GetBValue(cc) * best_bright);
				set_key(buf, KEYMAP[ki].offset, cr, cg, cb);
				ki++;
			}
			i = 0;
			while (i < MATRIX_COLUMNS)
			{
				col_pos[i] += col_speed[i] * speed_mul;
				if (col_pos[i] > 10)
				{
					col_pos[i] = -(double)(rand() % 6);
					col_speed[i] = 0.05 + (rand() % 100) / 300.0;
					col_color_idx[i] = rand() % color_count;
				}
				i++;
			}
		}
		else if (style == 2)
		{
			target = (int)g_matrixDensityTarget;
			if (target >= 4)
				laser_count = 3;
			else
				laser_count = 2;
			ki = 0;
			while (ki < KEYMAP_COUNT)
			{
				x = get_key_x(&KEYMAP[ki]);
				y = KEYMAP[ki].row;
				best_bright = 0;
				best_color = 0;
				s = 0;
				while (s < laser_count)
				{
					dir_x = cos(laser_angle[s]);
					dir_y = sin(laser_angle[s]);
					proj = x * dir_x + y * dir_y;
					dist = fabs(proj - laser_pos[s]);
					if (dist < 0.6)
					{
						b = 1.0 - dist / 0.6;
						if (b > best_bright)
						{
							best_bright = b;
							best_color = laser_color_idx[s] % color_count;
						}
					}
					s++;
				}
				if (best_bright > 1)
					best_bright = 1;
				col = g_matrixColors[best_color];
				cr = (unsigned char)(GetRValue(col) * best_bright);
				cg = (unsigned char)(GetGValue(col) * best_bright);
				cb = (unsigned char)(GetBValue(col) * best_bright);
				set_key(buf, KEYMAP[ki].offset, cr, cg, cb);
				ki++;
			}
			s = 0;
			while (s < laser_count)
			{
				laser_pos[s] += laser_spd[s] * speed_mul * 1.5;
				if (laser_pos[s] > 30)
				{
					laser_pos[s] = -30.0 - rand() % 20;
					laser_angle[s] = (rand() % 4) * (M_PI / 2.0);
					laser_spd[s] = 0.6 + (rand() % 100) / 100.0;
					laser_color_idx[s] = rand() % color_count;
				}
				s++;
			}
		}
		else
		{
			ki = 0;
			while (ki < KEYMAP_COUNT)
			{
				x = get_key_x(&KEYMAP[ki]);
				y = KEYMAP[ki].row;
				best_bright = 0;
				best_color = 0;
				s = 0;
				while (s < MAX_RIPPLES)
				{
					dx = x - ripple_x[s];
					dy = y - ripple_y[s];
					dist = sqrt(dx * dx + dy * dy);
					ring_dist = fabs(dist - ripple_r[s]);
					if (ring_dist < 1.2)
					{
						b = 1.0 - ring_dist / 1.2;
						if (b > best_bright)
						{
							best_bright = b;
							best_color = ripple_color_idx[s] % color_count;
						}
					}
					s++;
				}
				if (best_bright > 1)
					best_bright = 1;
				col = g_matrixColors[best_color];
				cr = (unsigned char)(GetRValue(col) * best_bright);
				cg = (unsigned char)(GetGValue(col) * best_bright);
				cb = (unsigned char)(GetBValue(col) * best_bright);
				set_key(buf, KEYMAP[ki].offset, cr, cg, cb);
				ki++;
			}
			s = 0;
			while (s < MAX_RIPPLES)
			{
				ripple_r[s] += speed_mul * 0.25;
				if (ripple_r[s] > 24)
				{
					ripple_x[s] = rand() % 20;
					ripple_y[s] = rand() % 7;
					ripple_r[s] = 0;
					ripple_color_idx[s] = rand() % color_count;
				}
				s++;
			}
		}
		send_frame(buf);
		Sleep(FRAME_DELAY_MS);
	}
	return (0);
}
