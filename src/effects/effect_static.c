#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include "effects.h"
#include "effect_common.h"
#include "device.h"
#include "keymap.h"
#include "config.h"

DWORD WINAPI	effect_static(LPVOID p)
{
	static int		*x_rank = NULL;
	static int		x_rank_ready = 0;
	int				idx[KEYMAP_COUNT];
	int				i;
	int				cur;
	double			cur_x;
	int				j;
	int				layout;
	int				max_zones;
	int				zone_count;
	unsigned char	buf[BUF_SIZE];
	size_t			ki;
	int				zone;
	COLORREF		c;

	(void)p;
	if (!x_rank_ready)
	{
		x_rank = (int *)malloc(KEYMAP_COUNT * sizeof(int));
		i = 0;
		while (i < (int)KEYMAP_COUNT)
		{
			idx[i] = i;
			i++;
		}
		i = 1;
		while (i < (int)KEYMAP_COUNT)
		{
			cur = idx[i];
			cur_x = get_key_x(&KEYMAP[cur]);
			j = i;
			while (j > 0 && get_key_x(&KEYMAP[idx[j - 1]]) > cur_x)
			{
				idx[j] = idx[j - 1];
				j--;
			}
			idx[j] = cur;
			i++;
		}
		i = 0;
		while (i < (int)KEYMAP_COUNT)
		{
			x_rank[idx[i]] = i;
			i++;
		}
		x_rank_ready = 1;
	}
	while (!g_stopFlag)
	{
		layout = (int)g_staticLayout;
		if (layout == 0)
			max_zones = 6;
		else
			max_zones = 10;
		zone_count = (int)g_staticZoneCount;
		if (zone_count < 1)
			zone_count = 1;
		if (zone_count > max_zones)
			zone_count = max_zones;
		memset(buf, 0, BUF_SIZE);
		ki = 0;
		while (ki < KEYMAP_COUNT)
		{
			zone = 0;
			if (zone_count > 1)
			{
				if (layout == 0)
					zone = horizontal_zone_for_row(KEYMAP[ki].row, zone_count);
				else
				{
					zone = (x_rank[ki] * zone_count) / (int)KEYMAP_COUNT;
					if (zone >= zone_count)
						zone = zone_count - 1;
				}
			}
			c = g_staticColors[zone];
			set_key(buf, KEYMAP[ki].offset, GetRValue(c), GetGValue(c), GetBValue(c));
			ki++;
		}
		send_frame(buf);
		Sleep(FRAME_DELAY_MS);
	}
	return (0);
}
