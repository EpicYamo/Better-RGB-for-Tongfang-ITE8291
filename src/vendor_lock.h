#ifndef VENDOR_LOCK_H
# define VENDOR_LOCK_H

extern int	g_startupLaunch;

int		is_elevated(void);
void	restart_vendor_service_if_found(void);
int		begin_relock(void);
int		relaunch_elevated(const wchar_t *args, int wait_for_exit);

#endif
