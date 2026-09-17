#ifndef EFFECT_COMMON_H
# define EFFECT_COMMON_H

# include <windows.h>

extern volatile LONG	g_stopFlag;
extern HANDLE			g_effectThread;

void	hsv_to_rgb(double h, unsigned char *r, unsigned char *g, unsigned char *b);
int		compute_prob_count(int target, int min_count, int max_count);
int		horizontal_zone_for_row(int row, int zone_count);

#endif
