#include <math.h>
#include <stdlib.h>
#include "effect_common.h"

void	hsv_to_rgb(double h, unsigned char *r, unsigned char *g, unsigned char *b)
{
	double	c;
	double	x;
	double	rr;
	double	gg;
	double	bb;

	h = fmod(h, 360.0);
	if (h < 0)
		h += 360.0;
	c = 1.0;
	x = c * (1 - fabs(fmod(h / 60.0, 2) - 1));
	if (h < 60)
	{
		rr = c;
		gg = x;
		bb = 0;
	}
	else if (h < 120)
	{
		rr = x;
		gg = c;
		bb = 0;
	}
	else if (h < 180)
	{
		rr = 0;
		gg = c;
		bb = x;
	}
	else if (h < 240)
	{
		rr = 0;
		gg = x;
		bb = c;
	}
	else if (h < 300)
	{
		rr = x;
		gg = 0;
		bb = c;
	}
	else
	{
		rr = c;
		gg = 0;
		bb = x;
	}
	*r = (unsigned char)(rr * 255);
	*g = (unsigned char)(gg * 255);
	*b = (unsigned char)(bb * 255);
}

int	compute_prob_count(int target, int min_count, int max_count)
{
	int	r;
	int	v;

	if (target >= max_count)
		return (max_count);
	r = rand() % 100;
	if (r < 25)
	{
		v = target - 1;
		if (v < min_count)
			return (min_count);
		return (v);
	}
	else if (r < 75)
		return (target);
	else
	{
		v = target + 1;
		if (v > max_count)
			return (max_count);
		return (v);
	}
}

int	horizontal_zone_for_row(int row, int zone_count)
{
	if (zone_count == 1)
		return (0);
	if (zone_count == 2)
	{
		if (row <= 3)
			return (0);
		return (1);
	}
	if (zone_count == 3)
	{
		if (row <= 1)
			return (0);
		if (row <= 4)
			return (1);
		return (2);
	}
	if (zone_count == 4)
	{
		if (row == 0)
			return (0);
		if (row <= 2)
			return (1);
		if (row <= 4)
			return (2);
		return (3);
	}
	if (zone_count == 5)
	{
		if (row == 0)
			return (0);
		if (row == 1)
			return (1);
		if (row == 2)
			return (2);
		if (row <= 4)
			return (3);
		return (4);
	}
	if (zone_count == 6)
	{
		if (row == 0)
			return (0);
		if (row == 1)
			return (1);
		if (row == 2)
			return (2);
		if (row == 3)
			return (3);
		if (row == 4)
			return (4);
		return (5);
	}
	return (0);
}
