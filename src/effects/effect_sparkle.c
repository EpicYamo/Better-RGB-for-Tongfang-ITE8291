#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include "effects.h"
#include "effect_common.h"
#include "device.h"
#include "keymap.h"
#include "config.h"

DWORD WINAPI	effect_sparkle(LPVOID p)
{
	int				*life;
	int				*max_life;
	unsigned char	*rr;
	unsigned char	*gg;
	unsigned char	*bb;
	int				target;
	int				speed;
	int				life_duration;
	int				alive_count;
	size_t			i;
	int				deficit;
	int				n;
	int				idx;
	int				attempt;
	int				c;
	unsigned char	buf[BUF_SIZE];
	double			fade;

	(void)p;
	life = (int *)calloc(KEYMAP_COUNT, sizeof(int));
	max_life = (int *)calloc(KEYMAP_COUNT, sizeof(int));
	rr = (unsigned char *)calloc(KEYMAP_COUNT, 1);
	gg = (unsigned char *)calloc(KEYMAP_COUNT, 1);
	bb = (unsigned char *)calloc(KEYMAP_COUNT, 1);
	while (!g_stopFlag)
	{
		target = (int)g_sparkleDensity;
		speed = (int)g_sparkleSpeed;
		life_duration = 50 - speed * 4;
		if (life_duration < 8)
			life_duration = 8;
		alive_count = 0;
		i = 0;
		while (i < KEYMAP_COUNT)
		{
			if (life[i] > 0)
				alive_count++;
			i++;
		}
		deficit = target - alive_count;
		n = 0;
		while (n < deficit)
		{
			idx = -1;
			attempt = 0;
			while (attempt < 20)
			{
				c = rand() % KEYMAP_COUNT;
				if (life[c] == 0)
				{
					idx = c;
					break;
				}
				attempt++;
			}
			if (idx < 0)
				break;
			life[idx] = life_duration;
			max_life[idx] = life_duration;
			rr[idx] = rand() % 256;
			gg[idx] = rand() % 256;
			bb[idx] = rand() % 256;
			n++;
		}
		memset(buf, 0, BUF_SIZE);
		i = 0;
		while (i < KEYMAP_COUNT)
		{
			if (life[i] > 0)
			{
				fade = life[i] / (double)max_life[i];
				set_key(buf, KEYMAP[i].offset, (unsigned char)(rr[i] * fade),
					(unsigned char)(gg[i] * fade), (unsigned char)(bb[i] * fade));
				life[i]--;
			}
			i++;
		}
		send_frame(buf);
		Sleep(FRAME_DELAY_MS);
	}
	free(life);
	free(max_life);
	free(rr);
	free(gg);
	free(bb);
	return (0);
}
