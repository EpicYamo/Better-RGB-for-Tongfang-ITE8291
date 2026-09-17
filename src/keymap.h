#ifndef KEYMAP_H
# define KEYMAP_H

# include <stddef.h>

typedef struct
{
	const char	*name;
	int			offset;
	int			row;
	int			col;
}	KeyEntry;

extern const KeyEntry	KEYMAP[];
extern const size_t	KEYMAP_COUNT;

int		vk_to_keymap_index(int vk);
double	get_key_x(const KeyEntry *k);

#endif
