#ifndef CONFIG_H
# define CONFIG_H

# include <windows.h>

# define MODE_COUNT	10

extern volatile LONG	g_activeMode;
extern volatile LONG	g_brightness;

extern COLORREF			g_breathColor;
extern volatile LONG	g_breathSpeed;
extern volatile LONG	g_breathSmooth;

extern volatile LONG	g_waveAngle;
extern volatile LONG	g_waveSpeed;

extern volatile LONG	g_sparkleSpeed;
extern volatile LONG	g_sparkleDensity;

extern COLORREF			g_reactiveColor;
extern volatile LONG	g_reactiveRandomMode;
extern volatile LONG	g_reactiveDuration;

extern volatile LONG	g_wheelReverse;
extern volatile LONG	g_wheelSpeed;

extern volatile LONG	g_lightningSpeed;
extern volatile LONG	g_lightningSmooth;
extern volatile LONG	g_lightningWidth;
extern volatile LONG	g_lightningConcurrent;

extern volatile LONG	g_flameSpeed;
extern volatile LONG	g_flameSmooth;

extern volatile LONG	g_rainSpeed;
extern volatile LONG	g_rainDensityTarget;
extern volatile LONG	g_rainColorCount;
extern COLORREF			g_rainColors[5];

extern volatile LONG	g_matrixSpeed;
extern volatile LONG	g_matrixStyle;
extern volatile LONG	g_matrixDensityTarget;
extern volatile LONG	g_matrixColorCount;
extern COLORREF			g_matrixColors[3];

extern volatile LONG	g_staticLayout;
extern volatile LONG	g_staticZoneCount;
extern COLORREF			g_staticColors[10];

extern LONG				g_cfgMode;

void	save_config(void);
void	load_config(void);

#endif
