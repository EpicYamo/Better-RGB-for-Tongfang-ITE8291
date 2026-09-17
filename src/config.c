#include <stdio.h>
#include <string.h>
#include "config.h"
#include "log.h"

volatile LONG	g_activeMode = -1;

COLORREF		g_breathColor = RGB(0, 120, 255);
volatile LONG	g_breathSpeed = 12;
volatile LONG	g_breathSmooth = 60;

volatile LONG	g_waveAngle = 0;
volatile LONG	g_waveSpeed = 4;

volatile LONG	g_sparkleSpeed = 5;
volatile LONG	g_sparkleDensity = 8;

COLORREF		g_reactiveColor = RGB(255, 255, 255);
volatile LONG	g_reactiveRandomMode = 1;
volatile LONG	g_reactiveDuration = 20;

volatile LONG	g_wheelReverse = 0;
volatile LONG	g_wheelSpeed = 4;

volatile LONG	g_lightningSpeed = 5;
volatile LONG	g_lightningSmooth = 10;
volatile LONG	g_lightningWidth = 2;
volatile LONG	g_lightningConcurrent = 1;

volatile LONG	g_flameSpeed = 5;
volatile LONG	g_flameSmooth = 8;

volatile LONG	g_rainSpeed = 5;
volatile LONG	g_rainDensityTarget = 3;
volatile LONG	g_rainColorCount = 1;
COLORREF		g_rainColors[5] = {
	RGB(80, 140, 255), RGB(120, 80, 255), RGB(80, 220, 255),
	RGB(255, 255, 255), RGB(80, 255, 180)
};

volatile LONG	g_matrixSpeed = 5;
volatile LONG	g_matrixStyle = 0;
volatile LONG	g_matrixDensityTarget = 2;
volatile LONG	g_matrixColorCount = 1;
COLORREF		g_matrixColors[3] = {
	RGB(0, 255, 60), RGB(0, 180, 255), RGB(255, 0, 120)
};

volatile LONG	g_staticLayout = 0;
volatile LONG	g_staticZoneCount = 1;
COLORREF		g_staticColors[10] = {
	RGB(255, 255, 255), RGB(255, 0, 0), RGB(0, 255, 0), RGB(0, 0, 255),
	RGB(255, 255, 0), RGB(255, 0, 255), RGB(0, 255, 255), RGB(255, 140, 0),
	RGB(140, 0, 255), RGB(0, 140, 140)
};

LONG	g_cfgMode = -1;

static char	g_cfgPath[MAX_PATH] = "";

static LONG	cfg_clamp(long v, long lo, long hi)
{
	if (v < lo)
		return ((LONG)lo);
	if (v > hi)
		return ((LONG)hi);
	return ((LONG)v);
}

void	save_config(void)
{
	FILE	*f;
	int		i;

	if (!g_cfgPath[0])
		build_sibling_path(g_cfgPath, "better_rgb.cfg");
	f = fopen(g_cfgPath, "w");
	if (!f)
	{
		log_event("save_config: could not open config file for writing");
		return ;
	}
	fprintf(f, "mode=%ld\n", (long)g_activeMode);
	fprintf(f, "brightness=%ld\n", (long)g_brightness);
	fprintf(f, "breathColor=%ld\n", (long)g_breathColor);
	fprintf(f, "breathSpeed=%ld\n", (long)g_breathSpeed);
	fprintf(f, "breathSmooth=%ld\n", (long)g_breathSmooth);
	fprintf(f, "waveAngle=%ld\n", (long)g_waveAngle);
	fprintf(f, "waveSpeed=%ld\n", (long)g_waveSpeed);
	fprintf(f, "sparkleSpeed=%ld\n", (long)g_sparkleSpeed);
	fprintf(f, "sparkleDensity=%ld\n", (long)g_sparkleDensity);
	fprintf(f, "reactiveColor=%ld\n", (long)g_reactiveColor);
	fprintf(f, "reactiveRandom=%ld\n", (long)g_reactiveRandomMode);
	fprintf(f, "reactiveDuration=%ld\n", (long)g_reactiveDuration);
	fprintf(f, "wheelReverse=%ld\n", (long)g_wheelReverse);
	fprintf(f, "wheelSpeed=%ld\n", (long)g_wheelSpeed);
	fprintf(f, "lightningSpeed=%ld\n", (long)g_lightningSpeed);
	fprintf(f, "lightningSmooth=%ld\n", (long)g_lightningSmooth);
	fprintf(f, "lightningWidth=%ld\n", (long)g_lightningWidth);
	fprintf(f, "lightningConcurrent=%ld\n", (long)g_lightningConcurrent);
	fprintf(f, "flameSpeed=%ld\n", (long)g_flameSpeed);
	fprintf(f, "flameSmooth=%ld\n", (long)g_flameSmooth);
	fprintf(f, "rainSpeed=%ld\n", (long)g_rainSpeed);
	fprintf(f, "rainDensity=%ld\n", (long)g_rainDensityTarget);
	fprintf(f, "rainColorCount=%ld\n", (long)g_rainColorCount);
	i = 0;
	while (i < 5)
	{
		fprintf(f, "rainColor%d=%ld\n", i, (long)g_rainColors[i]);
		i++;
	}
	fprintf(f, "matrixSpeed=%ld\n", (long)g_matrixSpeed);
	fprintf(f, "matrixStyle=%ld\n", (long)g_matrixStyle);
	fprintf(f, "matrixDensity=%ld\n", (long)g_matrixDensityTarget);
	fprintf(f, "matrixColorCount=%ld\n", (long)g_matrixColorCount);
	i = 0;
	while (i < 3)
	{
		fprintf(f, "matrixColor%d=%ld\n", i, (long)g_matrixColors[i]);
		i++;
	}
	fprintf(f, "staticLayout=%ld\n", (long)g_staticLayout);
	fprintf(f, "staticZones=%ld\n", (long)g_staticZoneCount);
	i = 0;
	while (i < 10)
	{
		fprintf(f, "staticColor%d=%ld\n", i, (long)g_staticColors[i]);
		i++;
	}
	fclose(f);
}

void	load_config(void)
{
	FILE	*f;
	char	key[64];
	long	v;

	if (!g_cfgPath[0])
		build_sibling_path(g_cfgPath, "better_rgb.cfg");
	f = fopen(g_cfgPath, "r");
	if (!f)
		return ;
	while (fscanf(f, " %63[^=]=%ld", key, &v) == 2)
	{
		if (!strcmp(key, "mode"))
			g_cfgMode = cfg_clamp(v, -1, MODE_COUNT - 1);
		else if (!strcmp(key, "brightness"))
			g_brightness = cfg_clamp(v, 0, 100);
		else if (!strcmp(key, "breathColor"))
			g_breathColor = (COLORREF)v;
		else if (!strcmp(key, "breathSpeed"))
			g_breathSpeed = cfg_clamp(v, 1, 40);
		else if (!strcmp(key, "breathSmooth"))
			g_breathSmooth = cfg_clamp(v, 2, 150);
		else if (!strcmp(key, "waveAngle"))
			g_waveAngle = cfg_clamp(v, 0, 359);
		else if (!strcmp(key, "waveSpeed"))
			g_waveSpeed = cfg_clamp(v, 1, 20);
		else if (!strcmp(key, "sparkleSpeed"))
			g_sparkleSpeed = cfg_clamp(v, 1, 10);
		else if (!strcmp(key, "sparkleDensity"))
			g_sparkleDensity = cfg_clamp(v, 3, 20);
		else if (!strcmp(key, "reactiveColor"))
			g_reactiveColor = (COLORREF)v;
		else if (!strcmp(key, "reactiveRandom"))
		{
			if (v)
				g_reactiveRandomMode = 1;
			else
				g_reactiveRandomMode = 0;
		}
		else if (!strcmp(key, "reactiveDuration"))
		{
			if (v == 10 || v == 45)
				g_reactiveDuration = (LONG)v;
			else
				g_reactiveDuration = 20;
		}
		else if (!strcmp(key, "wheelReverse"))
		{
			if (v)
				g_wheelReverse = 1;
			else
				g_wheelReverse = 0;
		}
		else if (!strcmp(key, "wheelSpeed"))
			g_wheelSpeed = cfg_clamp(v, 1, 20);
		else if (!strcmp(key, "lightningSpeed"))
			g_lightningSpeed = cfg_clamp(v, 1, 20);
		else if (!strcmp(key, "lightningSmooth"))
			g_lightningSmooth = cfg_clamp(v, 1, 40);
		else if (!strcmp(key, "lightningWidth"))
			g_lightningWidth = cfg_clamp(v, 1, 4);
		else if (!strcmp(key, "lightningConcurrent"))
			g_lightningConcurrent = cfg_clamp(v, 1, 4);
		else if (!strcmp(key, "flameSpeed"))
			g_flameSpeed = cfg_clamp(v, 1, 20);
		else if (!strcmp(key, "flameSmooth"))
			g_flameSmooth = cfg_clamp(v, 1, 40);
		else if (!strcmp(key, "rainSpeed"))
			g_rainSpeed = cfg_clamp(v, 1, 20);
		else if (!strcmp(key, "rainDensity"))
			g_rainDensityTarget = cfg_clamp(v, 1, 5);
		else if (!strcmp(key, "rainColorCount"))
			g_rainColorCount = cfg_clamp(v, 1, 5);
		else if (!strncmp(key, "rainColor", 9) && key[9] >= '0'
			&& key[9] <= '4' && !key[10])
			g_rainColors[key[9] - '0'] = (COLORREF)v;
		else if (!strcmp(key, "matrixSpeed"))
			g_matrixSpeed = cfg_clamp(v, 1, 20);
		else if (!strcmp(key, "matrixStyle"))
			g_matrixStyle = cfg_clamp(v, 0, 3);
		else if (!strcmp(key, "matrixDensity"))
			g_matrixDensityTarget = cfg_clamp(v, 1, 5);
		else if (!strcmp(key, "matrixColorCount"))
			g_matrixColorCount = cfg_clamp(v, 1, 3);
		else if (!strncmp(key, "matrixColor", 11) && key[11] >= '0'
			&& key[11] <= '2' && !key[12])
			g_matrixColors[key[11] - '0'] = (COLORREF)v;
		else if (!strcmp(key, "staticLayout"))
			g_staticLayout = cfg_clamp(v, 0, 1);
		else if (!strcmp(key, "staticZones"))
			g_staticZoneCount = cfg_clamp(v, 1, 10);
		else if (!strncmp(key, "staticColor", 11) && key[11] >= '0'
			&& key[11] <= '9' && !key[12])
			g_staticColors[key[11] - '0'] = (COLORREF)v;
	}
	fclose(f);
	log_event("load_config: settings restored from config file");
}
