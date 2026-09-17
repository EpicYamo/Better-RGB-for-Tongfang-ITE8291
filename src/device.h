#ifndef DEVICE_H
# define DEVICE_H

# include <windows.h>
# include "hidapi.h"

# define BUF_SIZE		512
# define FRAME_DELAY_MS	10

extern hid_device			*g_h;
extern CRITICAL_SECTION		g_deviceLock;
extern char					g_devicePath[512];
extern const GUID			GUID_DEVINTERFACE_HID_LOCAL;
extern volatile LONG		g_brightness;

int		resolve_device_path(void);
int		reconnect_device(void);
void	request_reconnect(const char *reason);
int		enter_custom_mode(void);
int		log_exclusive_status(void);
int		send_frame(unsigned char *buf);
void	set_key(unsigned char *buf, int offset, unsigned char r, unsigned char g, unsigned char b);

#endif
