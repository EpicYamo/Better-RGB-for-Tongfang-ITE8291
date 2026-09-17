#ifndef UI_DRAW_H
# define UI_DRAW_H

# include <windows.h>

# define CLR_BG			RGB(24, 24, 28)
# define CLR_PANEL		RGB(45, 45, 52)
# define CLR_PANEL_HOV	RGB(55, 55, 64)
# define CLR_ACCENT		RGB(127, 90, 240)
# define CLR_ACCENT_LT	RGB(170, 140, 255)
# define CLR_TEXT		RGB(235, 235, 240)
# define CLR_BORDER		RGB(70, 70, 80)

LRESULT CALLBACK	SwatchProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
void		set_swatch_color(HWND hwnd, COLORREF col);
COLORREF	pick_color(HWND hwnd, COLORREF initial);
void		draw_button_ex(LPDRAWITEMSTRUCT dis, BOOL use_custom_color, COLORREF custom_color);
void		draw_selector_button(LPDRAWITEMSTRUCT dis, BOOL is_active);
void		draw_button(LPDRAWITEMSTRUCT dis);
void		draw_labeled_color_button(LPDRAWITEMSTRUCT dis, COLORREF color);
BOOL CALLBACK	SetFontOnChild(HWND hwnd, LPARAM lfont);
HWND		mk_label(HWND parent, const wchar_t *text, int x, int y, int w, int h);
HWND		mk_slider(HWND parent, int id, int x, int y, int w, int h, int lo, int hi, int pos);
HWND		mk_button(HWND parent, const wchar_t *text, int id, int x, int y, int w, int h);

#endif
