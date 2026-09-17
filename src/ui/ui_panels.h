#ifndef UI_PANELS_H
# define UI_PANELS_H

# include <windows.h>

# define MAX_PANEL_CTRLS	25

extern HWND	g_panelBreath[MAX_PANEL_CTRLS];
extern int	g_panelBreathCount;
extern HWND	g_panelWave[MAX_PANEL_CTRLS];
extern int	g_panelWaveCount;
extern HWND	g_panelSparkle[MAX_PANEL_CTRLS];
extern int	g_panelSparkleCount;
extern HWND	g_panelReactive[MAX_PANEL_CTRLS];
extern int	g_panelReactiveCount;
extern HWND	g_panelWheel[MAX_PANEL_CTRLS];
extern int	g_panelWheelCount;
extern HWND	g_panelLightning[MAX_PANEL_CTRLS];
extern int	g_panelLightningCount;
extern HWND	g_panelFlame[MAX_PANEL_CTRLS];
extern int	g_panelFlameCount;
extern HWND	g_panelRain[MAX_PANEL_CTRLS];
extern int	g_panelRainCount;
extern HWND	g_panelMatrix[MAX_PANEL_CTRLS];
extern int	g_panelMatrixCount;
extern HWND	g_panelStatic[MAX_PANEL_CTRLS];
extern int	g_panelStaticCount;

extern HWND	g_rainColorBtn[5];
extern HWND	g_rainDensityBtn[5];
extern HWND	g_rainColorCountBtn[5];

extern HWND	g_matrixColorBtn[3];
extern HWND	g_matrixDensityBtn[5];
extern HWND	g_matrixColorCountBtn[3];
extern HWND	g_hComboMatrixStyle;

extern HWND	g_btnStaticHoriz;
extern HWND	g_btnStaticVert;
extern HWND	g_hSliderStaticZones;
extern HWND	g_hComboStaticZoneSel;
extern HWND	g_btnStaticColor;
extern volatile LONG	g_staticActiveZone;

void	show_only(HWND *arr, int count, BOOL show);
void	show_panel_for_mode(int mode);
void	refresh_rain_color_visibility(void);
void	refresh_matrix_color_visibility(void);
void	invalidate_group(HWND *arr, int count);
void	refresh_static_ui(void);

#endif
