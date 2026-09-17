#include <windows.h>
#include <string.h>
#include "keymap.h"

const KeyEntry	KEYMAP[] =
{
	{"Esc", 21, 0, 0}, {"F1", 45, 0, 1}, {"F2", 69, 0, 2}, {"F3", 93, 0, 3}, {"F4", 117, 0, 4},
	{"F5", 141, 0, 5}, {"F6", 165, 0, 6}, {"F7", 189, 0, 7}, {"F8", 213, 0, 8}, {"F9", 237, 0, 9},
	{"F10", 261, 0, 10}, {"F11", 285, 0, 11}, {"F12", 309, 0, 12}, {"FnLockCam", 333, 0, 13},
	{"PrtSc", 357, 0, 14}, {"Del", 381, 0, 15}, {"Home", 405, 0, 16}, {"PgUp", 429, 0, 17},
	{"PgDn", 453, 0, 18}, {"End", 477, 0, 19},
	{"Grave", 17, 1, 0}, {"1", 41, 1, 1}, {"2", 65, 1, 2}, {"3", 89, 1, 3}, {"4", 113, 1, 4},
	{"5", 137, 1, 5}, {"6", 161, 1, 6}, {"7", 185, 1, 7}, {"8", 209, 1, 8}, {"9", 233, 1, 9},
	{"0", 257, 1, 10}, {"Minus", 281, 1, 11}, {"Equals", 305, 1, 12}, {"Backspace", 353, 1, 13},
	{"NumLock", 377, 1, 14}, {"NumpadDiv", 401, 1, 15}, {"NumpadMul", 425, 1, 16}, {"NumpadMinus", 449, 1, 17},
	{"Tab", 13, 2, 0}, {"Q", 61, 2, 1}, {"W", 85, 2, 2}, {"E", 109, 2, 3}, {"R", 133, 2, 4},
	{"T", 157, 2, 5}, {"Y", 181, 2, 6}, {"U", 205, 2, 7}, {"I", 229, 2, 8}, {"O", 253, 2, 9},
	{"P", 277, 2, 10}, {"BracketL", 301, 2, 11}, {"BracketR", 325, 2, 12}, {"Enter", 349, 2, 13},
	{"Numpad7", 373, 2, 14}, {"Numpad8", 397, 2, 15}, {"Numpad9", 421, 2, 16}, {"NumpadPlus", 445, 2, 17},
	{"CapsLock", 9, 3, 0}, {"A", 57, 3, 1}, {"S", 81, 3, 2}, {"D", 105, 3, 3}, {"F", 129, 3, 4},
	{"G", 153, 3, 5}, {"H", 177, 3, 6}, {"J", 201, 3, 7}, {"K", 225, 3, 8}, {"L", 249, 3, 9},
	{"Semicolon", 273, 3, 10}, {"Quote", 297, 3, 11}, {"Backslash", 321, 3, 12},
	{"Numpad4", 369, 3, 13}, {"Numpad5", 393, 3, 14}, {"Numpad6", 417, 3, 15},
	{"ShiftL", 29, 4, 0}, {"ISO_Backslash", 53, 4, 1}, {"Z", 77, 4, 2}, {"X", 101, 4, 3},
	{"C", 125, 4, 4}, {"V", 149, 4, 5}, {"B", 173, 4, 6}, {"N", 197, 4, 7}, {"M", 221, 4, 8},
	{"Comma", 245, 4, 9}, {"Period", 269, 4, 10}, {"Slash", 293, 4, 11}, {"ShiftR", 341, 4, 12},
	{"Numpad1", 365, 4, 13}, {"Numpad2", 389, 4, 14}, {"Numpad3", 413, 4, 15}, {"NumpadEnter", 437, 4, 16},
	{"CtrlL", 1, 5, 0}, {"Fn", 49, 5, 1}, {"Windows", 73, 5, 2}, {"AltL", 97, 5, 3},
	{"Space", 169, 5, 4}, {"AltGr", 241, 5, 5}, {"CopilotKey", 289, 5, 6}, {"ArrowUp", 337, 5, 7},
	{"Numpad0", 385, 5, 8}, {"NumpadDecimal", 409, 5, 9},
	{"ArrowLeft", 313, 6, 6}, {"ArrowDown", 433, 6, 7}, {"ArrowRight", 361, 6, 8},
};

const size_t	KEYMAP_COUNT = sizeof(KEYMAP) / sizeof(KEYMAP[0]);

int	vk_to_keymap_index(int vk)
{
	struct	{ int vk; const char *name; }	table[] =
	{
		{VK_SPACE, "Space"}, {VK_RETURN, "Enter"}, {VK_BACK, "Backspace"}, {VK_TAB, "Tab"},
		{VK_LSHIFT, "ShiftL"}, {VK_RSHIFT, "ShiftR"}, {VK_CAPITAL, "CapsLock"}, {VK_LCONTROL, "CtrlL"},
		{VK_LMENU, "AltL"}, {VK_RMENU, "AltGr"}, {VK_LEFT, "ArrowLeft"}, {VK_RIGHT, "ArrowRight"},
		{VK_UP, "ArrowUp"}, {VK_DOWN, "ArrowDown"}, {VK_OEM_102, "ISO_Backslash"}, {VK_LWIN, "Windows"},
		{VK_OEM_4, "BracketL"}, {VK_OEM_6, "BracketR"}, {VK_OEM_1, "Semicolon"}, {VK_OEM_7, "Quote"},
		{VK_OEM_5, "Backslash"}, {VK_OEM_COMMA, "Comma"}, {VK_OEM_PERIOD, "Period"}, {VK_OEM_2, "Slash"},
		{VK_OEM_MINUS, "Minus"}, {VK_OEM_PLUS, "Equals"}, {VK_OEM_3, "Grave"},
		{VK_ESCAPE, "Esc"}, {VK_F1, "F1"}, {VK_F2, "F2"}, {VK_F3, "F3"}, {VK_F4, "F4"}, {VK_F5, "F5"},
		{VK_F6, "F6"}, {VK_F7, "F7"}, {VK_F8, "F8"}, {VK_F9, "F9"}, {VK_F10, "F10"}, {VK_F11, "F11"}, {VK_F12, "F12"},
		{VK_SNAPSHOT, "PrtSc"}, {VK_DELETE, "Del"}, {VK_HOME, "Home"}, {VK_PRIOR, "PgUp"}, {VK_NEXT, "PgDn"}, {VK_END, "End"},
		{VK_NUMPAD0, "Numpad0"}, {VK_NUMPAD1, "Numpad1"}, {VK_NUMPAD2, "Numpad2"}, {VK_NUMPAD3, "Numpad3"},
		{VK_NUMPAD4, "Numpad4"}, {VK_NUMPAD5, "Numpad5"}, {VK_NUMPAD6, "Numpad6"}, {VK_NUMPAD7, "Numpad7"},
		{VK_NUMPAD8, "Numpad8"}, {VK_NUMPAD9, "Numpad9"}, {VK_DECIMAL, "NumpadDecimal"}, {VK_ADD, "NumpadPlus"},
		{VK_SUBTRACT, "NumpadMinus"}, {VK_MULTIPLY, "NumpadMul"}, {VK_DIVIDE, "NumpadDiv"}
	};
	size_t	i;
	size_t	k;

	i = 0;
	while (i < KEYMAP_COUNT)
	{
		if (((vk >= 'A' && vk <= 'Z') || (vk >= '0' && vk <= '9'))
			&& KEYMAP[i].name[0] == (char)vk && KEYMAP[i].name[1] == '\0')
			return ((int)i);
		i++;
	}
	k = 0;
	while (k < sizeof(table) / sizeof(table[0]))
	{
		if (vk == table[k].vk)
		{
			i = 0;
			while (i < KEYMAP_COUNT)
			{
				if (strcmp(KEYMAP[i].name, table[k].name) == 0)
					return ((int)i);
				i++;
			}
		}
		k++;
	}
	return (-1);
}

double	get_key_x(const KeyEntry *k)
{
	const double	gap = 1.5;

	if (strcmp(k->name, "AltGr") == 0)
		return (10.5);
	if (strcmp(k->name, "CopilotKey") == 0)
		return (11.5);
	if (strcmp(k->name, "ArrowUp") == 0)
		return (13.0);
	if (strcmp(k->name, "ArrowLeft") == 0)
		return (12.0);
	if (strcmp(k->name, "ArrowDown") == 0)
		return (13.0);
	if (strcmp(k->name, "ArrowRight") == 0)
		return (14.0);
	if (strcmp(k->name, "Numpad0") == 0)
		return (15.0);
	if (strcmp(k->name, "NumpadDecimal") == 0)
		return (16.0);
	if (strncmp(k->name, "Numpad", 6) == 0 || strcmp(k->name, "NumLock") == 0)
		return (k->col + gap);
	if (strcmp(k->name, "FnLockCam") == 0 || strcmp(k->name, "PrtSc") == 0
		|| strcmp(k->name, "Del") == 0 || strcmp(k->name, "Home") == 0
		|| strcmp(k->name, "PgUp") == 0 || strcmp(k->name, "PgDn") == 0
		|| strcmp(k->name, "End") == 0)
		return (k->col + gap);
	return ((double)k->col);
}
