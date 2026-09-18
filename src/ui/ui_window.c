#include <windows.h>
#include <commctrl.h>
#include <dbt.h>
#include <wchar.h>
#include <stdio.h>
#include <string.h>
#include "resource.h"
#include "ui_window.h"
#include "ui_draw.h"
#include "ui_panels.h"
#include "effects.h"
#include "config.h"
#include "device.h"
#include "vendor_lock.h"
#include "autostart.h"
#include "log.h"

HWND	g_hStatus = NULL;
HWND	g_hComboMode = NULL;
HWND	g_btnAutostart = NULL;
HBRUSH	g_bgBrush = NULL;

static HWND				g_hSliderBright;
static HWND				g_hLblAngle;
static HWND				g_hLblConcurrent;
static HWND				g_hSwatchBreath;
static HWND				g_hSwatchReact;
static NOTIFYICONDATA	g_nid;
static UINT				g_msgTaskbarCreated;
static UINT				g_msgShowInstance;
static int				g_trayRetries = 0;

static const wchar_t	*const g_digitLabels5[5] = {L"1", L"2", L"3", L"4", L"5"};
static const wchar_t	*const g_digitLabels3[3] = {L"1", L"2", L"3"};

LRESULT CALLBACK	WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	if (g_msgTaskbarCreated && msg == g_msgTaskbarCreated)
	{
		if (Shell_NotifyIcon(NIM_ADD, &g_nid))
		{
			KillTimer(hwnd, IDT_TRAY_RETRY);
			log_event("tray icon re-added after TaskbarCreated broadcast");
		}
		return (0);
	}
	if (g_msgShowInstance && msg == g_msgShowInstance)
	{
		ShowWindow(hwnd, SW_SHOW);
		ShowWindow(hwnd, SW_RESTORE);
		SetForegroundWindow(hwnd);
		log_event("second launch detected, window restored");
		return (0);
	}
	switch (msg)
	{
		case WM_CREATE:
		{
			HICON	hAppIcon;
			HFONT	hFont;
			HWND	sAngle;
			HWND	rShort;
			HWND	rMedium;
			HWND	rLong;
			HWND	chkRev;
			HWND	btnStaticVert;
			int		c;

			hAppIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_APPICON));
			if (!hAppIcon)
				hAppIcon = LoadIcon(NULL, IDI_APPLICATION);
			memset(&g_nid, 0, sizeof(g_nid));
			g_nid.cbSize = sizeof(g_nid);
			g_nid.hWnd = hwnd;
			g_nid.uID = 1;
			g_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
			g_nid.uCallbackMessage = WM_TRAYICON;
			g_nid.hIcon = hAppIcon;
			wcscpy(g_nid.szTip, L"Better RGB by TheYamo");
			g_msgTaskbarCreated = RegisterWindowMessageW(L"TaskbarCreated");
			ChangeWindowMessageFilterEx(hwnd, g_msgTaskbarCreated, MSGFLT_ALLOW, NULL);
			g_msgShowInstance = RegisterWindowMessageW(L"BetterRGB_TheYamo_Show");
			ChangeWindowMessageFilterEx(hwnd, g_msgShowInstance, MSGFLT_ALLOW, NULL);
			if (!Shell_NotifyIcon(NIM_ADD, &g_nid))
			{
				log_event("tray icon add failed (taskbar not ready yet), scheduling retries");
				g_trayRetries = 0;
				SetTimer(hwnd, IDT_TRAY_RETRY, 2000, NULL);
			}
			hFont = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
					DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
					CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");

			mk_label(hwnd, L"Select Mode:", 20, 20, 100, 20);
			g_hComboMode = CreateWindow(L"COMBOBOX", L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
					130, 18, 230, 200, hwnd, (HMENU)IDC_COMBO_MODE, NULL, NULL);
			SendMessage(g_hComboMode, CB_ADDSTRING, 0, (LPARAM)L"Breathing");
			SendMessage(g_hComboMode, CB_ADDSTRING, 0, (LPARAM)L"Wave");
			SendMessage(g_hComboMode, CB_ADDSTRING, 0, (LPARAM)L"Sparkle");
			SendMessage(g_hComboMode, CB_ADDSTRING, 0, (LPARAM)L"Reactive");
			SendMessage(g_hComboMode, CB_ADDSTRING, 0, (LPARAM)L"Wheel");
			SendMessage(g_hComboMode, CB_ADDSTRING, 0, (LPARAM)L"Lightning");
			SendMessage(g_hComboMode, CB_ADDSTRING, 0, (LPARAM)L"Flame");
			SendMessage(g_hComboMode, CB_ADDSTRING, 0, (LPARAM)L"Rain");
			SendMessage(g_hComboMode, CB_ADDSTRING, 0, (LPARAM)L"Matrix");
			SendMessage(g_hComboMode, CB_ADDSTRING, 0, (LPARAM)L"Static");
			mk_button(hwnd, L"Stop (Turn Off Lights)", IDC_BTN_STOP, 20, 55, 340, 35);

			g_panelBreath[g_panelBreathCount++] = mk_label(hwnd, L"Breathing Color:", 20, 105, 150, 20);
			g_hSwatchBreath = CreateWindow(L"SwatchClass", L"", WS_CHILD | WS_BORDER,
					20, 128, 50, 28, hwnd, (HMENU)(INT_PTR)999, NULL, NULL);
			g_panelBreath[g_panelBreathCount++] = g_hSwatchBreath;
			g_panelBreath[g_panelBreathCount++] = mk_button(hwnd, L"Pick Color", IDC_BTN_COLOR_BREATH, 80, 128, 120, 28);
			g_panelBreath[g_panelBreathCount++] = mk_label(hwnd, L"Speed:", 20, 165, 150, 20);
			g_panelBreath[g_panelBreathCount++] = mk_slider(hwnd, IDC_SLIDER_BSPEED, 20, 185, 340, 30, 1, 40, g_breathSpeed);
			g_panelBreath[g_panelBreathCount++] = mk_label(hwnd, L"Smoothness:", 20, 220, 150, 20);
			g_panelBreath[g_panelBreathCount++] = mk_slider(hwnd, IDC_SLIDER_BSMOOTH, 20, 240, 340, 30, 2, 150, g_breathSmooth);

			g_hLblAngle = mk_label(hwnd, L"Angle: 0 degrees", 20, 105, 300, 20);
			g_panelWave[g_panelWaveCount++] = g_hLblAngle;
			sAngle = mk_slider(hwnd, IDC_SLIDER_ANGLE, 20, 125, 340, 30, 0, 359, g_waveAngle);
			SendMessage(sAngle, TBM_SETTICFREQ, 45, 0);
			SetWindowLongPtr(sAngle, GWL_STYLE, GetWindowLongPtr(sAngle, GWL_STYLE) | TBS_TOOLTIPS);
			SendMessage(sAngle, TBM_SETTIPSIDE, TBTS_TOP, 0);
			g_panelWave[g_panelWaveCount++] = sAngle;
			g_panelWave[g_panelWaveCount++] = mk_label(hwnd, L"Speed:", 20, 165, 150, 20);
			g_panelWave[g_panelWaveCount++] = mk_slider(hwnd, IDC_SLIDER_WSPEED, 20, 185, 340, 30, 1, 20, g_waveSpeed);

			g_panelSparkle[g_panelSparkleCount++] = mk_label(hwnd, L"Speed:", 20, 105, 150, 20);
			g_panelSparkle[g_panelSparkleCount++] = mk_slider(hwnd, IDC_SLIDER_SPSPEED, 20, 125, 340, 30, 1, 10, g_sparkleSpeed);
			g_panelSparkle[g_panelSparkleCount++] = mk_label(hwnd, L"Density:", 20, 165, 150, 20);
			g_panelSparkle[g_panelSparkleCount++] = mk_slider(hwnd, IDC_SLIDER_SPDENS, 20, 185, 340, 30, 3, 20, g_sparkleDensity);

			g_panelReactive[g_panelReactiveCount++] = mk_label(hwnd, L"Reactive Color:", 20, 105, 200, 20);
			g_hSwatchReact = CreateWindow(L"SwatchClass", L"", WS_CHILD | WS_BORDER,
					20, 128, 50, 28, hwnd, (HMENU)(INT_PTR)998, NULL, NULL);
			g_panelReactive[g_panelReactiveCount++] = g_hSwatchReact;
			g_panelReactive[g_panelReactiveCount++] = mk_button(hwnd, L"Pick Color", IDC_BTN_COLOR_REACT, 80, 128, 120, 28);
			g_panelReactive[g_panelReactiveCount++] = mk_button(hwnd, L"Random", IDC_BTN_REACT_RANDOM, 210, 128, 90, 28);
			g_panelReactive[g_panelReactiveCount++] = mk_label(hwnd, L"Duration:", 20, 168, 150, 20);
			rShort = CreateWindow(L"BUTTON", L"Short", WS_CHILD | BS_AUTORADIOBUTTON | WS_GROUP,
					20, 190, 100, 24, hwnd, (HMENU)IDC_RADIO_SHORT, NULL, NULL);
			rMedium = CreateWindow(L"BUTTON", L"Medium", WS_CHILD | BS_AUTORADIOBUTTON,
					130, 190, 100, 24, hwnd, (HMENU)IDC_RADIO_MEDIUM, NULL, NULL);
			rLong = CreateWindow(L"BUTTON", L"Long", WS_CHILD | BS_AUTORADIOBUTTON,
					240, 190, 100, 24, hwnd, (HMENU)IDC_RADIO_LONG, NULL, NULL);
			SendMessage(rMedium, BM_SETCHECK, BST_CHECKED, 0);
			g_panelReactive[g_panelReactiveCount++] = rShort;
			g_panelReactive[g_panelReactiveCount++] = rMedium;
			g_panelReactive[g_panelReactiveCount++] = rLong;

			chkRev = CreateWindow(L"BUTTON", L"Reverse Direction", WS_CHILD | BS_AUTOCHECKBOX,
					20, 105, 200, 20, hwnd, (HMENU)IDC_CHK_WHEEL_REVERSE, NULL, NULL);
			g_panelWheel[g_panelWheelCount++] = chkRev;
			g_panelWheel[g_panelWheelCount++] = mk_label(hwnd, L"Speed:", 20, 135, 150, 20);
			g_panelWheel[g_panelWheelCount++] = mk_slider(hwnd, IDC_SLIDER_WHSPEED, 20, 155, 340, 30, 1, 20, g_wheelSpeed);

			g_panelLightning[g_panelLightningCount++] = mk_label(hwnd, L"Speed:", 20, 105, 200, 20);
			g_panelLightning[g_panelLightningCount++] = mk_slider(hwnd, IDC_SLIDER_LTSPEED, 20, 125, 340, 30, 1, 10, g_lightningSpeed);
			g_panelLightning[g_panelLightningCount++] = mk_label(hwnd, L"Smoothness:", 20, 165, 200, 20);
			g_panelLightning[g_panelLightningCount++] = mk_slider(hwnd, IDC_SLIDER_LTSMOOTH, 20, 185, 340, 30, 1, 20, g_lightningSmooth);
			g_panelLightning[g_panelLightningCount++] = mk_label(hwnd, L"Strike Width:", 20, 225, 200, 20);
			g_panelLightning[g_panelLightningCount++] = mk_slider(hwnd, IDC_SLIDER_LTWIDTH, 20, 245, 340, 30, 1, 4, g_lightningWidth);
			g_hLblConcurrent = mk_label(hwnd, L"Concurrent Strikes: 1", 20, 285, 250, 20);
			g_panelLightning[g_panelLightningCount++] = g_hLblConcurrent;
			g_panelLightning[g_panelLightningCount++] = mk_slider(hwnd, IDC_SLIDER_LTCONCUR, 20, 305, 340, 30, 1, 4, g_lightningConcurrent);

			g_panelFlame[g_panelFlameCount++] = mk_label(hwnd, L"Flicker Speed:", 20, 105, 200, 20);
			g_panelFlame[g_panelFlameCount++] = mk_slider(hwnd, IDC_SLIDER_FLSPEED, 20, 125, 340, 30, 1, 10, g_flameSpeed);
			g_panelFlame[g_panelFlameCount++] = mk_label(hwnd, L"Smoothness:", 20, 165, 200, 20);
			g_panelFlame[g_panelFlameCount++] = mk_slider(hwnd, IDC_SLIDER_FLSMOOTH, 20, 185, 340, 30, 1, 20, g_flameSmooth);

			g_panelRain[g_panelRainCount++] = mk_label(hwnd, L"Fall Speed:", 20, 105, 200, 20);
			g_panelRain[g_panelRainCount++] = mk_slider(hwnd, IDC_SLIDER_RNSPEED, 20, 125, 340, 30, 1, 10, g_rainSpeed);
			g_panelRain[g_panelRainCount++] = mk_label(hwnd, L"Density (simultaneous drops):", 20, 165, 300, 20);
			c = 0;
			while (c < 5)
			{
				g_rainDensityBtn[c] = mk_button(hwnd, g_digitLabels5[c], IDC_RAIN_DENSITY_BASE + c, 20 + c * 68, 185, 60, 30);
				g_panelRain[g_panelRainCount++] = g_rainDensityBtn[c];
				c++;
			}
			g_panelRain[g_panelRainCount++] = mk_label(hwnd, L"Color Count:", 20, 225, 200, 20);
			c = 0;
			while (c < 5)
			{
				g_rainColorCountBtn[c] = mk_button(hwnd, g_digitLabels5[c], IDC_RAIN_COLORCNT_BASE + c, 20 + c * 68, 245, 60, 30);
				g_panelRain[g_panelRainCount++] = g_rainColorCountBtn[c];
				c++;
			}
			c = 0;
			while (c < 5)
			{
				g_rainColorBtn[c] = mk_button(hwnd, L"", IDC_RAIN_COLOR_BASE + c, 20 + c * 68, 285, 60, 32);
				g_panelRain[g_panelRainCount++] = g_rainColorBtn[c];
				c++;
			}

			g_panelMatrix[g_panelMatrixCount++] = mk_label(hwnd, L"Streak Speed:", 20, 105, 200, 20);
			g_panelMatrix[g_panelMatrixCount++] = mk_slider(hwnd, IDC_SLIDER_MXSPEED, 20, 125, 340, 30, 1, 10, g_matrixSpeed);
			g_panelMatrix[g_panelMatrixCount++] = mk_label(hwnd, L"Style:", 20, 165, 200, 20);
			g_hComboMatrixStyle = CreateWindow(L"COMBOBOX", L"", WS_CHILD | CBS_DROPDOWNLIST | WS_VSCROLL,
					20, 185, 340, 100, hwnd, (HMENU)IDC_COMBO_MXSTYLE, NULL, NULL);
			SendMessage(g_hComboMatrixStyle, CB_ADDSTRING, 0, (LPARAM)L"Diagonal Streaks");
			SendMessage(g_hComboMatrixStyle, CB_ADDSTRING, 0, (LPARAM)L"Vertical Columns");
			SendMessage(g_hComboMatrixStyle, CB_ADDSTRING, 0, (LPARAM)L"Laser Sweep");
			SendMessage(g_hComboMatrixStyle, CB_ADDSTRING, 0, (LPARAM)L"Ripple");
			SendMessage(g_hComboMatrixStyle, CB_SETCURSEL, 0, 0);
			g_panelMatrix[g_panelMatrixCount++] = g_hComboMatrixStyle;
			g_panelMatrix[g_panelMatrixCount++] = mk_label(hwnd, L"Density (Diagonal/Laser modes):", 20, 220, 300, 20);
			c = 0;
			while (c < 5)
			{
				g_matrixDensityBtn[c] = mk_button(hwnd, g_digitLabels5[c], IDC_MATRIX_DENSITY_BASE + c, 20 + c * 68, 240, 60, 30);
				g_panelMatrix[g_panelMatrixCount++] = g_matrixDensityBtn[c];
				c++;
			}
			g_panelMatrix[g_panelMatrixCount++] = mk_label(hwnd, L"Color Count:", 20, 280, 200, 20);
			c = 0;
			while (c < 3)
			{
				g_matrixColorCountBtn[c] = mk_button(hwnd, g_digitLabels3[c], IDC_MATRIX_COLORCNT_BASE + c, 20 + c * 113, 300, 105, 30);
				g_panelMatrix[g_panelMatrixCount++] = g_matrixColorCountBtn[c];
				c++;
			}
			c = 0;
			while (c < 3)
			{
				g_matrixColorBtn[c] = mk_button(hwnd, L"", IDC_MATRIX_COLOR_BASE + c, 20 + c * 113, 340, 105, 32);
				g_panelMatrix[g_panelMatrixCount++] = g_matrixColorBtn[c];
				c++;
			}

			g_panelStatic[g_panelStaticCount++] = mk_label(hwnd, L"Layout:", 20, 105, 200, 20);
			g_btnStaticHoriz = mk_button(hwnd, L"Horizontal", IDC_BTN_STATIC_HORIZ, 20, 125, 165, 32);
			btnStaticVert = mk_button(hwnd, L"Vertical", IDC_BTN_STATIC_VERT, 195, 125, 165, 32);
			g_btnStaticVert = btnStaticVert;
			g_panelStatic[g_panelStaticCount++] = g_btnStaticHoriz;
			g_panelStatic[g_panelStaticCount++] = g_btnStaticVert;
			g_panelStatic[g_panelStaticCount++] = mk_label(hwnd, L"Zone Count:", 20, 165, 200, 20);
			g_hSliderStaticZones = mk_slider(hwnd, IDC_SLIDER_STATIC_ZONES, 20, 185, 340, 30, 1, 6, g_staticZoneCount);
			g_panelStatic[g_panelStaticCount++] = g_hSliderStaticZones;
			g_hComboStaticZoneSel = CreateWindow(L"COMBOBOX", L"", WS_CHILD | CBS_DROPDOWNLIST | WS_VSCROLL,
					20, 225, 340, 150, hwnd, (HMENU)IDC_COMBO_STATIC_ZONESEL, NULL, NULL);
			g_panelStatic[g_panelStaticCount++] = g_hComboStaticZoneSel;
			g_btnStaticColor = mk_button(hwnd, L"Select Color", IDC_BTN_STATIC_COLOR, 20, 265, 340, 36);
			g_panelStatic[g_panelStaticCount++] = g_btnStaticColor;

			mk_label(hwnd, L"Brightness:", 20, 400, 150, 20);
			g_hSliderBright = mk_slider(hwnd, IDC_SLIDER_BRIGHT, 20, 420, 340, 30, 0, 100, g_brightness);

			g_hStatus = CreateWindow(L"STATIC", L"Active Mode: None", WS_CHILD | WS_VISIBLE | SS_LEFT,
					20, 465, 340, 25, hwnd, (HMENU)IDC_STATIC_STATUS, NULL, NULL);

			g_btnAutostart = mk_button(hwnd, L"", IDC_BTN_AUTOSTART, 20, 497, 340, 30);
			update_autostart_button();

			set_swatch_color(g_hSwatchBreath, g_breathColor);
			set_swatch_color(g_hSwatchReact, g_reactiveColor);

			refresh_rain_color_visibility();
			refresh_matrix_color_visibility();
			refresh_static_ui();
			show_panel_for_mode(-1);

			EnumChildWindows(hwnd, SetFontOnChild, (LPARAM)hFont);
			return (0);
		}

		case WM_DRAWITEM:
		{
			LPDRAWITEMSTRUCT	dis;
			int					id;

			dis = (LPDRAWITEMSTRUCT)lp;
			id = dis->CtlID;
			if (id >= IDC_RAIN_COLOR_BASE && id < IDC_RAIN_COLOR_BASE + 5)
				draw_button_ex(dis, TRUE, g_rainColors[id - IDC_RAIN_COLOR_BASE]);
			else if (id >= IDC_MATRIX_COLOR_BASE && id < IDC_MATRIX_COLOR_BASE + 3)
				draw_button_ex(dis, TRUE, g_matrixColors[id - IDC_MATRIX_COLOR_BASE]);
			else if (id >= IDC_RAIN_DENSITY_BASE && id < IDC_RAIN_DENSITY_BASE + 5)
				draw_selector_button(dis, (id - IDC_RAIN_DENSITY_BASE + 1) == (int)g_rainDensityTarget);
			else if (id >= IDC_RAIN_COLORCNT_BASE && id < IDC_RAIN_COLORCNT_BASE + 5)
				draw_selector_button(dis, (id - IDC_RAIN_COLORCNT_BASE + 1) == (int)g_rainColorCount);
			else if (id >= IDC_MATRIX_DENSITY_BASE && id < IDC_MATRIX_DENSITY_BASE + 5)
				draw_selector_button(dis, (id - IDC_MATRIX_DENSITY_BASE + 1) == (int)g_matrixDensityTarget);
			else if (id >= IDC_MATRIX_COLORCNT_BASE && id < IDC_MATRIX_COLORCNT_BASE + 3)
				draw_selector_button(dis, (id - IDC_MATRIX_COLORCNT_BASE + 1) == (int)g_matrixColorCount);
			else if (id == IDC_BTN_STATIC_HORIZ)
				draw_selector_button(dis, g_staticLayout == 0);
			else if (id == IDC_BTN_STATIC_VERT)
				draw_selector_button(dis, g_staticLayout == 1);
			else if (id == IDC_BTN_STATIC_COLOR)
				draw_labeled_color_button(dis, g_staticColors[g_staticActiveZone]);
			else
				draw_button(dis);
			return (TRUE);
		}

		case WM_CTLCOLORSTATIC:
		{
			HDC	hdc;

			hdc = (HDC)wp;
			SetTextColor(hdc, CLR_TEXT);
			SetBkMode(hdc, TRANSPARENT);
			return ((LRESULT)g_bgBrush);
		}

		case WM_HSCROLL:
		{
			HWND	ctrl;
			wchar_t	buf[64];

			ctrl = (HWND)lp;
			if (ctrl == g_hSliderBright)
				g_brightness = (LONG)SendMessage(ctrl, TBM_GETPOS, 0, 0);
			else if (GetDlgCtrlID(ctrl) == IDC_SLIDER_BSPEED)
				g_breathSpeed = (LONG)SendMessage(ctrl, TBM_GETPOS, 0, 0);
			else if (GetDlgCtrlID(ctrl) == IDC_SLIDER_BSMOOTH)
				g_breathSmooth = (LONG)SendMessage(ctrl, TBM_GETPOS, 0, 0);
			else if (GetDlgCtrlID(ctrl) == IDC_SLIDER_ANGLE)
			{
				g_waveAngle = (LONG)SendMessage(ctrl, TBM_GETPOS, 0, 0);
				wsprintf(buf, L"Angle: %d degrees", (int)g_waveAngle);
				SetWindowText(g_hLblAngle, buf);
			}
			else if (GetDlgCtrlID(ctrl) == IDC_SLIDER_WSPEED)
				g_waveSpeed = (LONG)SendMessage(ctrl, TBM_GETPOS, 0, 0);
			else if (GetDlgCtrlID(ctrl) == IDC_SLIDER_SPSPEED)
				g_sparkleSpeed = (LONG)SendMessage(ctrl, TBM_GETPOS, 0, 0);
			else if (GetDlgCtrlID(ctrl) == IDC_SLIDER_SPDENS)
				g_sparkleDensity = (LONG)SendMessage(ctrl, TBM_GETPOS, 0, 0);
			else if (GetDlgCtrlID(ctrl) == IDC_SLIDER_WHSPEED)
				g_wheelSpeed = (LONG)SendMessage(ctrl, TBM_GETPOS, 0, 0);
			else if (GetDlgCtrlID(ctrl) == IDC_SLIDER_LTSPEED)
				g_lightningSpeed = (LONG)SendMessage(ctrl, TBM_GETPOS, 0, 0);
			else if (GetDlgCtrlID(ctrl) == IDC_SLIDER_LTSMOOTH)
				g_lightningSmooth = (LONG)SendMessage(ctrl, TBM_GETPOS, 0, 0);
			else if (GetDlgCtrlID(ctrl) == IDC_SLIDER_LTWIDTH)
				g_lightningWidth = (LONG)SendMessage(ctrl, TBM_GETPOS, 0, 0);
			else if (GetDlgCtrlID(ctrl) == IDC_SLIDER_LTCONCUR)
			{
				g_lightningConcurrent = (LONG)SendMessage(ctrl, TBM_GETPOS, 0, 0);
				wsprintf(buf, L"Concurrent Strikes: %d", (int)g_lightningConcurrent);
				SetWindowText(g_hLblConcurrent, buf);
			}
			else if (GetDlgCtrlID(ctrl) == IDC_SLIDER_FLSPEED)
				g_flameSpeed = (LONG)SendMessage(ctrl, TBM_GETPOS, 0, 0);
			else if (GetDlgCtrlID(ctrl) == IDC_SLIDER_FLSMOOTH)
				g_flameSmooth = (LONG)SendMessage(ctrl, TBM_GETPOS, 0, 0);
			else if (GetDlgCtrlID(ctrl) == IDC_SLIDER_RNSPEED)
				g_rainSpeed = (LONG)SendMessage(ctrl, TBM_GETPOS, 0, 0);
			else if (GetDlgCtrlID(ctrl) == IDC_SLIDER_MXSPEED)
				g_matrixSpeed = (LONG)SendMessage(ctrl, TBM_GETPOS, 0, 0);
			else if (GetDlgCtrlID(ctrl) == IDC_SLIDER_STATIC_ZONES)
			{
				g_staticZoneCount = (LONG)SendMessage(ctrl, TBM_GETPOS, 0, 0);
				refresh_static_ui();
			}
			return (0);
		}

		case WM_ACTIVATE:
			if (LOWORD(wp) != WA_INACTIVE)
				update_autostart_button();
			return (0);

		case WM_COMMAND:
			if (HIWORD(wp) == CBN_SELCHANGE && (HWND)lp == g_hComboMode)
			{
				int	sel;

				sel = (int)SendMessage(g_hComboMode, CB_GETCURSEL, 0, 0);
				if (sel >= 0)
				{
					show_panel_for_mode(sel);
					if (sel == MODE_RAIN)
						refresh_rain_color_visibility();
					if (sel == MODE_MATRIX)
						refresh_matrix_color_visibility();
					if (sel == MODE_STATIC)
						refresh_static_ui();
					start_mode(sel);
				}
				return (0);
			}
			if (HIWORD(wp) == CBN_SELCHANGE && (HWND)lp == g_hComboMatrixStyle)
			{
				g_matrixStyle = (LONG)SendMessage(g_hComboMatrixStyle, CB_GETCURSEL, 0, 0);
				return (0);
			}
			if (HIWORD(wp) == CBN_SELCHANGE && (HWND)lp == g_hComboStaticZoneSel)
			{
				g_staticActiveZone = (LONG)SendMessage(g_hComboStaticZoneSel, CB_GETCURSEL, 0, 0);
				InvalidateRect(g_btnStaticColor, NULL, TRUE);
				return (0);
			}
			{
				int	cid;

				cid = LOWORD(wp);
				if (cid >= IDC_RAIN_COLOR_BASE && cid < IDC_RAIN_COLOR_BASE + 5)
				{
					int	idx;

					idx = cid - IDC_RAIN_COLOR_BASE;
					g_rainColors[idx] = pick_color(hwnd, g_rainColors[idx]);
					InvalidateRect(g_rainColorBtn[idx], NULL, TRUE);
					return (0);
				}
				if (cid >= IDC_MATRIX_COLOR_BASE && cid < IDC_MATRIX_COLOR_BASE + 3)
				{
					int	idx;

					idx = cid - IDC_MATRIX_COLOR_BASE;
					g_matrixColors[idx] = pick_color(hwnd, g_matrixColors[idx]);
					InvalidateRect(g_matrixColorBtn[idx], NULL, TRUE);
					return (0);
				}
				if (cid >= IDC_RAIN_DENSITY_BASE && cid < IDC_RAIN_DENSITY_BASE + 5)
				{
					g_rainDensityTarget = cid - IDC_RAIN_DENSITY_BASE + 1;
					invalidate_group(g_rainDensityBtn, 5);
					return (0);
				}
				if (cid >= IDC_RAIN_COLORCNT_BASE && cid < IDC_RAIN_COLORCNT_BASE + 5)
				{
					g_rainColorCount = cid - IDC_RAIN_COLORCNT_BASE + 1;
					invalidate_group(g_rainColorCountBtn, 5);
					refresh_rain_color_visibility();
					return (0);
				}
				if (cid >= IDC_MATRIX_DENSITY_BASE && cid < IDC_MATRIX_DENSITY_BASE + 5)
				{
					g_matrixDensityTarget = cid - IDC_MATRIX_DENSITY_BASE + 1;
					invalidate_group(g_matrixDensityBtn, 5);
					return (0);
				}
				if (cid >= IDC_MATRIX_COLORCNT_BASE && cid < IDC_MATRIX_COLORCNT_BASE + 3)
				{
					g_matrixColorCount = cid - IDC_MATRIX_COLORCNT_BASE + 1;
					invalidate_group(g_matrixColorCountBtn, 3);
					refresh_matrix_color_visibility();
					return (0);
				}
				if (cid == IDC_BTN_STATIC_HORIZ)
				{
					g_staticLayout = 0;
					refresh_static_ui();
					return (0);
				}
				if (cid == IDC_BTN_STATIC_VERT)
				{
					g_staticLayout = 1;
					refresh_static_ui();
					return (0);
				}
				if (cid == IDC_BTN_STATIC_COLOR)
				{
					g_staticColors[g_staticActiveZone] = pick_color(hwnd, g_staticColors[g_staticActiveZone]);
					InvalidateRect(g_btnStaticColor, NULL, TRUE);
					return (0);
				}
			}
			switch (LOWORD(wp))
			{
				case IDC_BTN_AUTOSTART:
				{
					int	exists;

					exists = autostart_task_exists();
					if (is_elevated())
					{
						if (exists)
							autostart_apply(0);
						else
							autostart_apply(1);
					}
					else
					{
						if (exists)
							relaunch_elevated(L"--remove-autostart", 0);
						else
							relaunch_elevated(L"--install-autostart", 0);
					}
					update_autostart_button();
					return (0);
				}
				case IDC_BTN_STOP:
					stop_current_effect();
					set_status(L"Active Mode: None");
					show_panel_for_mode(-1);
					SendMessage(g_hComboMode, CB_SETCURSEL, (WPARAM)-1, 0);
					save_config();
					break ;
				case IDC_BTN_COLOR_BREATH:
					g_breathColor = pick_color(hwnd, g_breathColor);
					set_swatch_color(g_hSwatchBreath, g_breathColor);
					break ;
				case IDC_BTN_COLOR_REACT:
					g_reactiveColor = pick_color(hwnd, g_reactiveColor);
					set_swatch_color(g_hSwatchReact, g_reactiveColor);
					g_reactiveRandomMode = 0;
					break ;
				case IDC_BTN_REACT_RANDOM:
					g_reactiveRandomMode = 1;
					set_swatch_color(g_hSwatchReact, RGB(200, 200, 200));
					break ;
				case IDC_CHK_WHEEL_REVERSE:
					if (IsDlgButtonChecked(hwnd, IDC_CHK_WHEEL_REVERSE) == BST_CHECKED)
						g_wheelReverse = 1;
					else
						g_wheelReverse = 0;
					break ;
				case IDC_RADIO_SHORT:
					g_reactiveDuration = 10;
					break ;
				case IDC_RADIO_MEDIUM:
					g_reactiveDuration = 20;
					break ;
				case IDC_RADIO_LONG:
					g_reactiveDuration = 45;
					break ;
				case IDC_TRAY_RESTORE:
					ShowWindow(hwnd, SW_SHOW);
					ShowWindow(hwnd, SW_RESTORE);
					SetForegroundWindow(hwnd);
					break ;
				case IDC_TRAY_EXIT:
					DestroyWindow(hwnd);
					break ;
			}
			return (0);

		case WM_TIMER:
			if (wp == IDT_TRAY_RETRY)
			{
				if (Shell_NotifyIcon(NIM_ADD, &g_nid))
				{
					KillTimer(hwnd, IDT_TRAY_RETRY);
					log_event("tray icon added on retry");
				}
				else
				{
					g_trayRetries++;
					if (g_trayRetries >= 30)
					{
						KillTimer(hwnd, IDT_TRAY_RETRY);
						log_event("tray icon retries exhausted");
					}
				}
			}
			return (0);

		case WM_SYSCOMMAND:
			if ((wp & 0xFFF0) == SC_MINIMIZE)
			{
				ShowWindow(hwnd, SW_HIDE);
				return (0);
			}
			return (DefWindowProc(hwnd, msg, wp, lp));

		case WM_CLOSE:
			ShowWindow(hwnd, SW_HIDE);
			return (0);

		case WM_TRAYICON:
			if (lp == WM_LBUTTONDBLCLK || lp == WM_LBUTTONUP)
			{
				ShowWindow(hwnd, SW_SHOW);
				ShowWindow(hwnd, SW_RESTORE);
				SetForegroundWindow(hwnd);
			}
			else if (lp == WM_RBUTTONUP)
			{
				POINT	pt;
				HMENU	menu;

				GetCursorPos(&pt);
				menu = CreatePopupMenu();
				AppendMenu(menu, MF_STRING, IDC_TRAY_RESTORE, L"Open");
				AppendMenu(menu, MF_SEPARATOR, 0, NULL);
				AppendMenu(menu, MF_STRING, IDC_TRAY_EXIT, L"Exit");
				SetForegroundWindow(hwnd);
				TrackPopupMenu(menu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
				DestroyMenu(menu);
			}
			return (0);

		case WM_DEVICECHANGE:
		{
			char	logbuf[128];

			if (wp == DBT_DEVICEARRIVAL || wp == DBT_DEVICEREMOVECOMPLETE)
			{
				sprintf(logbuf, "WM_DEVICECHANGE received, wParam=%lu", (unsigned long)wp);
				log_event(logbuf);
				request_reconnect("device-change");
			}
			return (TRUE);
		}

		case WM_POWERBROADCAST:
		{
			char	logbuf[128];

			sprintf(logbuf, "WM_POWERBROADCAST received, wParam=%lu", (unsigned long)wp);
			log_event(logbuf);
			request_reconnect("power-broadcast");
			return (TRUE);
		}

		case WM_ENDSESSION:
			if (wp)
				save_config();
			return (0);

		case WM_DESTROY:
			KillTimer(hwnd, IDT_TRAY_RETRY);
			save_config();
			stop_current_effect();
			Shell_NotifyIcon(NIM_DELETE, &g_nid);
			PostQuitMessage(0);
			return (0);
	}
	return (DefWindowProc(hwnd, msg, wp, lp));
}
