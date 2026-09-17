#include <windows.h>
#include <commctrl.h>
#include "ui_panels.h"
#include "effects.h"
#include "config.h"

HWND	g_panelBreath[MAX_PANEL_CTRLS];
int		g_panelBreathCount = 0;
HWND	g_panelWave[MAX_PANEL_CTRLS];
int		g_panelWaveCount = 0;
HWND	g_panelSparkle[MAX_PANEL_CTRLS];
int		g_panelSparkleCount = 0;
HWND	g_panelReactive[MAX_PANEL_CTRLS];
int		g_panelReactiveCount = 0;
HWND	g_panelWheel[MAX_PANEL_CTRLS];
int		g_panelWheelCount = 0;
HWND	g_panelLightning[MAX_PANEL_CTRLS];
int		g_panelLightningCount = 0;
HWND	g_panelFlame[MAX_PANEL_CTRLS];
int		g_panelFlameCount = 0;
HWND	g_panelRain[MAX_PANEL_CTRLS];
int		g_panelRainCount = 0;
HWND	g_panelMatrix[MAX_PANEL_CTRLS];
int		g_panelMatrixCount = 0;
HWND	g_panelStatic[MAX_PANEL_CTRLS];
int		g_panelStaticCount = 0;

HWND	g_rainColorBtn[5];
HWND	g_rainDensityBtn[5];
HWND	g_rainColorCountBtn[5];

HWND	g_matrixColorBtn[3];
HWND	g_matrixDensityBtn[5];
HWND	g_matrixColorCountBtn[3];
HWND	g_hComboMatrixStyle;

HWND			g_btnStaticHoriz;
HWND			g_btnStaticVert;
HWND			g_hSliderStaticZones;
HWND			g_hComboStaticZoneSel;
HWND			g_btnStaticColor;
volatile LONG	g_staticActiveZone = 0;

void	show_only(HWND *arr, int count, BOOL show)
{
	int	i;

	i = 0;
	while (i < count)
	{
		if (show)
			ShowWindow(arr[i], SW_SHOW);
		else
			ShowWindow(arr[i], SW_HIDE);
		i++;
	}
}

void	show_panel_for_mode(int mode)
{
	HWND	top;

	show_only(g_panelBreath, g_panelBreathCount, mode == MODE_BREATH);
	show_only(g_panelWave, g_panelWaveCount, mode == MODE_WAVE);
	show_only(g_panelSparkle, g_panelSparkleCount, mode == MODE_SPARKLE);
	show_only(g_panelReactive, g_panelReactiveCount, mode == MODE_REACTIVE);
	show_only(g_panelWheel, g_panelWheelCount, mode == MODE_WHEEL);
	show_only(g_panelLightning, g_panelLightningCount, mode == MODE_LIGHTNING);
	show_only(g_panelFlame, g_panelFlameCount, mode == MODE_FLAME);
	show_only(g_panelRain, g_panelRainCount, mode == MODE_RAIN);
	show_only(g_panelMatrix, g_panelMatrixCount, mode == MODE_MATRIX);
	show_only(g_panelStatic, g_panelStaticCount, mode == MODE_STATIC);
	top = GetAncestor(g_panelBreath[0], GA_ROOT);
	if (top)
	{
		InvalidateRect(top, NULL, TRUE);
		UpdateWindow(top);
	}
}

void	refresh_rain_color_visibility(void)
{
	int		i;
	HWND	top;

	i = 0;
	while (i < 5)
	{
		if (i < (int)g_rainColorCount)
			ShowWindow(g_rainColorBtn[i], SW_SHOW);
		else
			ShowWindow(g_rainColorBtn[i], SW_HIDE);
		i++;
	}
	top = GetAncestor(g_rainColorBtn[0], GA_ROOT);
	if (top)
	{
		InvalidateRect(top, NULL, TRUE);
		UpdateWindow(top);
	}
}

void	refresh_matrix_color_visibility(void)
{
	int		i;
	HWND	top;

	i = 0;
	while (i < 3)
	{
		if (i < (int)g_matrixColorCount)
			ShowWindow(g_matrixColorBtn[i], SW_SHOW);
		else
			ShowWindow(g_matrixColorBtn[i], SW_HIDE);
		i++;
	}
	top = GetAncestor(g_matrixColorBtn[0], GA_ROOT);
	if (top)
	{
		InvalidateRect(top, NULL, TRUE);
		UpdateWindow(top);
	}
}

void	invalidate_group(HWND *arr, int count)
{
	int	i;

	i = 0;
	while (i < count)
	{
		InvalidateRect(arr[i], NULL, TRUE);
		i++;
	}
}

void	refresh_static_ui(void)
{
	int		layout;
	int		max_zones;
	int		zc;
	int		i;
	wchar_t	buf[16];
	HWND	top;

	layout = (int)g_staticLayout;
	if (layout == 0)
		max_zones = 6;
	else
		max_zones = 10;
	zc = (int)g_staticZoneCount;
	if (zc > max_zones)
		zc = max_zones;
	if (zc < 1)
		zc = 1;
	g_staticZoneCount = zc;
	SendMessage(g_hSliderStaticZones, TBM_SETRANGE, TRUE, MAKELONG(1, max_zones));
	SendMessage(g_hSliderStaticZones, TBM_SETPOS, TRUE, zc);
	SendMessage(g_hComboStaticZoneSel, CB_RESETCONTENT, 0, 0);
	i = 0;
	while (i < zc)
	{
		wsprintf(buf, L"Color %d", i + 1);
		SendMessage(g_hComboStaticZoneSel, CB_ADDSTRING, 0, (LPARAM)buf);
		i++;
	}
	if (g_staticActiveZone >= zc)
		g_staticActiveZone = 0;
	SendMessage(g_hComboStaticZoneSel, CB_SETCURSEL, g_staticActiveZone, 0);
	if (zc > 1)
		ShowWindow(g_hComboStaticZoneSel, SW_SHOW);
	else
		ShowWindow(g_hComboStaticZoneSel, SW_HIDE);
	InvalidateRect(g_btnStaticColor, NULL, TRUE);
	InvalidateRect(g_btnStaticHoriz, NULL, TRUE);
	InvalidateRect(g_btnStaticVert, NULL, TRUE);
	top = GetAncestor(g_btnStaticColor, GA_ROOT);
	if (top)
	{
		InvalidateRect(top, NULL, TRUE);
		UpdateWindow(top);
	}
}
