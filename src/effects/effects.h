#ifndef EFFECTS_H
# define EFFECTS_H

# include <windows.h>

# define MODE_BREATH		0
# define MODE_WAVE			1
# define MODE_SPARKLE		2
# define MODE_REACTIVE		3
# define MODE_WHEEL			4
# define MODE_LIGHTNING		5
# define MODE_FLAME			6
# define MODE_RAIN			7
# define MODE_MATRIX		8
# define MODE_STATIC		9

extern HWND	g_hStatus;

DWORD WINAPI	effect_breathing(LPVOID p);
DWORD WINAPI	effect_wave(LPVOID p);
DWORD WINAPI	effect_wheel(LPVOID p);
DWORD WINAPI	effect_lightning(LPVOID p);
DWORD WINAPI	effect_flame(LPVOID p);
DWORD WINAPI	effect_rain(LPVOID p);
DWORD WINAPI	effect_matrix(LPVOID p);
DWORD WINAPI	effect_static(LPVOID p);
DWORD WINAPI	effect_sparkle(LPVOID p);
DWORD WINAPI	effect_reactive(LPVOID p);

void	set_status(const wchar_t *text);
void	stop_current_effect(void);
void	start_mode(int mode);

#endif
