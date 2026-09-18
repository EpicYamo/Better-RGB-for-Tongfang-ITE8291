#ifndef AUTOSTART_H
# define AUTOSTART_H

# include <windows.h>

extern HWND	g_btnAutostart;

int		autostart_task_exists(void);
void	autostart_apply(int install);
void	update_autostart_button(void);

#endif
