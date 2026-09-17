#include <windows.h>
#include <commctrl.h>
#include "ui_draw.h"

LRESULT CALLBACK	SwatchProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	PAINTSTRUCT	ps;
	HDC			hdc;
	RECT		rc;
	COLORREF	col;
	HBRUSH		br;

	if (msg == WM_PAINT)
	{
		hdc = BeginPaint(hwnd, &ps);
		GetClientRect(hwnd, &rc);
		col = (COLORREF)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		br = CreateSolidBrush(col);
		FillRect(hdc, &rc, br);
		DeleteObject(br);
		FrameRect(hdc, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH));
		EndPaint(hwnd, &ps);
		return (0);
	}
	return (DefWindowProc(hwnd, msg, wp, lp));
}

void	set_swatch_color(HWND hwnd, COLORREF col)
{
	SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)col);
	InvalidateRect(hwnd, NULL, TRUE);
}

COLORREF	pick_color(HWND hwnd, COLORREF initial)
{
	static COLORREF	custom[16] = {0};
	CHOOSECOLOR		cc = {0};

	cc.lStructSize = sizeof(cc);
	cc.hwndOwner = hwnd;
	cc.rgbResult = initial;
	cc.lpCustColors = custom;
	cc.Flags = CC_FULLOPEN | CC_RGBINIT;
	if (ChooseColor(&cc))
		return (cc.rgbResult);
	return (initial);
}

void	draw_button_ex(LPDRAWITEMSTRUCT dis, BOOL use_custom_color, COLORREF custom_color)
{
	HDC			hdc;
	RECT		rc;
	wchar_t		text[64];
	BOOL		pressed;
	COLORREF	bg;
	HBRUSH		br;
	HPEN		pen;
	HGDIOBJ		old_br;
	HGDIOBJ		old_pen;

	hdc = dis->hDC;
	rc = dis->rcItem;
	GetWindowText(dis->hwndItem, text, 64);
	pressed = (dis->itemState & ODS_SELECTED) != 0;
	if (use_custom_color)
		bg = custom_color;
	else if (pressed)
		bg = CLR_PANEL_HOV;
	else
		bg = CLR_PANEL;
	br = CreateSolidBrush(bg);
	pen = CreatePen(PS_SOLID, 1, CLR_BORDER);
	old_br = SelectObject(hdc, br);
	old_pen = SelectObject(hdc, pen);
	RoundRect(hdc, rc.left, rc.top, rc.right, rc.bottom, 10, 10);
	SelectObject(hdc, old_br);
	SelectObject(hdc, old_pen);
	DeleteObject(br);
	DeleteObject(pen);
	if (!use_custom_color)
	{
		SetBkMode(hdc, TRANSPARENT);
		SetTextColor(hdc, CLR_TEXT);
		DrawText(hdc, text, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	}
}

void	draw_selector_button(LPDRAWITEMSTRUCT dis, BOOL is_active)
{
	HDC			hdc;
	RECT		rc;
	wchar_t		text[64];
	COLORREF	bg;
	COLORREF	border;
	int			pen_width;
	HBRUSH		br;
	HPEN		pen;
	HGDIOBJ		old_br;
	HGDIOBJ		old_pen;

	hdc = dis->hDC;
	rc = dis->rcItem;
	GetWindowText(dis->hwndItem, text, 64);
	if (is_active)
	{
		bg = CLR_ACCENT;
		border = CLR_ACCENT_LT;
		pen_width = 2;
	}
	else
	{
		bg = CLR_PANEL;
		border = CLR_BORDER;
		pen_width = 1;
	}
	br = CreateSolidBrush(bg);
	pen = CreatePen(PS_SOLID, pen_width, border);
	old_br = SelectObject(hdc, br);
	old_pen = SelectObject(hdc, pen);
	RoundRect(hdc, rc.left, rc.top, rc.right, rc.bottom, 8, 8);
	SelectObject(hdc, old_br);
	SelectObject(hdc, old_pen);
	DeleteObject(br);
	DeleteObject(pen);
	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, CLR_TEXT);
	DrawText(hdc, text, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

void	draw_button(LPDRAWITEMSTRUCT dis)
{
	draw_button_ex(dis, FALSE, 0);
}

void	draw_labeled_color_button(LPDRAWITEMSTRUCT dis, COLORREF color)
{
	HDC			hdc;
	RECT		rc;
	wchar_t		text[64];
	HBRUSH		br;
	HPEN		pen;
	HGDIOBJ		old_br;
	HGDIOBJ		old_pen;
	double		lum;
	COLORREF	text_color;

	hdc = dis->hDC;
	rc = dis->rcItem;
	GetWindowText(dis->hwndItem, text, 64);
	br = CreateSolidBrush(color);
	pen = CreatePen(PS_SOLID, 1, CLR_BORDER);
	old_br = SelectObject(hdc, br);
	old_pen = SelectObject(hdc, pen);
	RoundRect(hdc, rc.left, rc.top, rc.right, rc.bottom, 10, 10);
	SelectObject(hdc, old_br);
	SelectObject(hdc, old_pen);
	DeleteObject(br);
	DeleteObject(pen);
	lum = 0.299 * GetRValue(color) + 0.587 * GetGValue(color) + 0.114 * GetBValue(color);
	if (lum > 140)
		text_color = RGB(0, 0, 0);
	else
		text_color = RGB(255, 255, 255);
	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, text_color);
	DrawText(hdc, text, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

BOOL CALLBACK	SetFontOnChild(HWND hwnd, LPARAM lfont)
{
	SendMessage(hwnd, WM_SETFONT, (WPARAM)lfont, TRUE);
	return (TRUE);
}

HWND	mk_label(HWND parent, const wchar_t *text, int x, int y, int w, int h)
{
	return (CreateWindow(L"STATIC", text, WS_CHILD | WS_VISIBLE,
			x, y, w, h, parent, NULL, NULL, NULL));
}

HWND	mk_slider(HWND parent, int id, int x, int y, int w, int h, int lo, int hi, int pos)
{
	HWND	s;

	s = CreateWindow(TRACKBAR_CLASS, L"", WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS,
			x, y, w, h, parent, (HMENU)(INT_PTR)id, NULL, NULL);
	SendMessage(s, TBM_SETRANGE, TRUE, MAKELONG(lo, hi));
	SendMessage(s, TBM_SETPOS, TRUE, pos);
	return (s);
}

HWND	mk_button(HWND parent, const wchar_t *text, int id, int x, int y, int w, int h)
{
	return (CreateWindow(L"BUTTON", text, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
			x, y, w, h, parent, (HMENU)(INT_PTR)id, NULL, NULL));
}
