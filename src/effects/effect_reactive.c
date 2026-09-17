#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include "effects.h"
#include "effect_common.h"
#include "device.h"
#include "keymap.h"
#include "config.h"

DWORD WINAPI	effect_reactive(LPVOID p)
{
	int				*was_down;
	int				*fade_life;
	unsigned char	*rr;
	unsigned char	*gg;
	unsigned char	*bb;
	int				vk_list[128];
	int				vk_count;
	int				vk;
	int				specials[] = {
		VK_SPACE, VK_RETURN, VK_BACK, VK_TAB, VK_LSHIFT, VK_RSHIFT, VK_CAPITAL,
		VK_LCONTROL, VK_LMENU, VK_RMENU, VK_LEFT, VK_RIGHT, VK_UP, VK_DOWN,
		VK_OEM_102, VK_LWIN, VK_OEM_4, VK_OEM_6, VK_OEM_1, VK_OEM_7, VK_OEM_5,
		VK_OEM_COMMA, VK_OEM_PERIOD, VK_OEM_2, VK_OEM_MINUS, VK_OEM_PLUS,
		VK_OEM_3, VK_ESCAPE, VK_F1, VK_F2, VK_F3, VK_F4, VK_F5, VK_F6, VK_F7,
		VK_F8, VK_F9, VK_F10, VK_F11, VK_F12, VK_SNAPSHOT, VK_DELETE, VK_HOME,
		VK_PRIOR, VK_NEXT, VK_END, VK_NUMPAD0, VK_NUMPAD1, VK_NUMPAD2,
		VK_NUMPAD3, VK_NUMPAD4, VK_NUMPAD5, VK_NUMPAD6, VK_NUMPAD7, VK_NUMPAD8,
		VK_NUMPAD9, VK_DECIMAL, VK_ADD, VK_SUBTRACT, VK_MULTIPLY, VK_DIVIDE
	};
	size_t			s;
	unsigned char	fr;
	unsigned char	fg;
	unsigned char	fb;
	int				v;
	int				idx;
	int				down;
	unsigned char	buf[BUF_SIZE];
	size_t			i;
	double			fade;

	(void)p;
	was_down = (int *)calloc(KEYMAP_COUNT, sizeof(int));
	fade_life = (int *)calloc(KEYMAP_COUNT, sizeof(int));
	rr = (unsigned char *)calloc(KEYMAP_COUNT, 1);
	gg = (unsigned char *)calloc(KEYMAP_COUNT, 1);
	bb = (unsigned char *)calloc(KEYMAP_COUNT, 1);
	vk_count = 0;
	vk = 'A';
	while (vk <= 'Z')
	{
		vk_list[vk_count] = vk;
		vk_count++;
		vk++;
	}
	vk = '0';
	while (vk <= '9')
	{
		vk_list[vk_count] = vk;
		vk_count++;
		vk++;
	}
	s = 0;
	while (s < sizeof(specials) / sizeof(specials[0]))
	{
		vk_list[vk_count] = specials[s];
		vk_count++;
		s++;
	}
	while (!g_stopFlag)
	{
		fr = GetRValue(g_reactiveColor);
		fg = GetGValue(g_reactiveColor);
		fb = GetBValue(g_reactiveColor);
		v = 0;
		while (v < vk_count)
		{
			idx = vk_to_keymap_index(vk_list[v]);
			if (idx < 0)
			{
				v++;
				continue;
			}
			down = (GetAsyncKeyState(vk_list[v]) & 0x8000) != 0;
			if (down)
			{
				if (!was_down[idx])
				{
					if (g_reactiveRandomMode)
					{
						rr[idx] = rand() % 256;
						gg[idx] = rand() % 256;
						bb[idx] = rand() % 256;
					}
					else
					{
						rr[idx] = fr;
						gg[idx] = fg;
						bb[idx] = fb;
					}
				}
				was_down[idx] = 1;
				fade_life[idx] = (int)g_reactiveDuration;
			}
			else
				was_down[idx] = 0;
			v++;
		}
		memset(buf, 0, BUF_SIZE);
		i = 0;
		while (i < KEYMAP_COUNT)
		{
			if (was_down[i])
				set_key(buf, KEYMAP[i].offset, rr[i], gg[i], bb[i]);
			else if (fade_life[i] > 0)
			{
				fade = fade_life[i] / (double)g_reactiveDuration;
				set_key(buf, KEYMAP[i].offset, (unsigned char)(rr[i] * fade),
					(unsigned char)(gg[i] * fade), (unsigned char)(bb[i] * fade));
				fade_life[i]--;
			}
			i++;
		}
		send_frame(buf);
		Sleep(FRAME_DELAY_MS);
	}
	free(was_down);
	free(fade_life);
	free(rr);
	free(gg);
	free(bb);
	return (0);
}
