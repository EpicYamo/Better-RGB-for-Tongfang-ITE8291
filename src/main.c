#include <windows.h>
#include <dbt.h>
#include <commctrl.h>
#include <stdlib.h>
#include <wchar.h>
#include <time.h>
#include <stdio.h>
#include "hidapi.h"
#include "resource.h"
#include "ui_window.h"
#include "ui_draw.h"
#include "ui_panels.h"
#include "effects.h"
#include "config.h"
#include "device.h"
#include "vendor_lock.h"
#include "autostart.h"
#include "log.h"

int	g_startupLaunch = 0;

int WINAPI	WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR cmd, int show)
{
	LPWSTR							cl;
	int								relockMode;
	DWORD							relockParentPid;
	wchar_t							*rl;
	HANDLE							hParent;
	HANDLE							hSingleInstance;
	UINT							showMsg;
	HWND							prev;
	int								excl;
	int								attempt;
	char							lb[128];
	int								ans;
	int								ans2;
	INITCOMMONCONTROLSEX			icc;
	WNDCLASS						swc;
	WNDCLASS						wc;
	HWND							hwnd;
	int								durationRadioId;
	HPOWERNOTIFY					hPowerNotify;
	DEV_BROADCAST_DEVICEINTERFACE	devFilter;
	HDEVNOTIFY						hDevNotify;
	MSG								msg;

	(void)hPrev;
	(void)cmd;
	(void)show;
	srand((unsigned int)time(NULL));
	cl = GetCommandLineW();
	if (wcsstr(cl, L"--install-autostart"))
	{
		autostart_apply(1);
		return (0);
	}
	if (wcsstr(cl, L"--remove-autostart"))
	{
		autostart_apply(0);
		return (0);
	}
	g_startupLaunch = (wcsstr(cl, L"--startup") != NULL);
	load_config();
	relockMode = 0;
	relockParentPid = 0;
	rl = wcsstr(cl, L"--relock");
	if (rl)
	{
		relockMode = 1;
		relockParentPid = (DWORD)wcstoul(rl + 8, NULL, 10);
	}
	if (relockMode && relockParentPid)
	{
		hParent = OpenProcess(SYNCHRONIZE, FALSE, relockParentPid);
		if (hParent)
		{
			WaitForSingleObject(hParent, 5000);
			CloseHandle(hParent);
		}
		Sleep(150);
	}
	hSingleInstance = CreateMutexW(NULL, TRUE, L"BetterRGB_TheYamo_SingleInstance");
	if (hSingleInstance && GetLastError() == ERROR_ALREADY_EXISTS)
	{
		showMsg = RegisterWindowMessageW(L"BetterRGB_TheYamo_Show");
		prev = FindWindowW(L"RGBMainWindowClass", NULL);
		if (prev)
		{
			PostMessageW(prev, showMsg, 0, 0);
			ShowWindow(prev, SW_SHOW);
			ShowWindow(prev, SW_RESTORE);
			SetForegroundWindow(prev);
		}
		else
			PostMessageW(HWND_BROADCAST, showMsg, 0, 0);
		CloseHandle(hSingleInstance);
		return (0);
	}
	InitializeCriticalSection(&g_deviceLock);
	hid_init();
	SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
	if (!resolve_device_path())
	{
		if (relockMode)
			restart_vendor_service_if_found();
		MessageBoxW(NULL,
			L"No compatible keyboard controller was found on this system.\n"
			L"This app targets ITE8291-based keyboards (USB vendor ID 048D) with a per-key RGB interface.\n"
			L"Details of every ITE device that was checked are in rgb_engine_log.txt next to the exe - please share that file when reporting this.\n\n"
			L"Bu sistemde uyumlu bir klavye kontrolcusu bulunamadi.\n"
			L"Bu uygulama, per-key RGB arayuzune sahip ITE8291 tabanli klavyeleri (USB vendor ID 048D) hedefler.\n"
			L"Kontrol edilen tum ITE cihazlarinin detaylari exe'nin yanindaki rgb_engine_log.txt dosyasindadir - sorunu bildirirken lutfen bu dosyayi paylasin.",
			L"Better RGB - Error / Hata", MB_OK | MB_ICONERROR);
		return (1);
	}
	g_h = hid_open_path(g_devicePath);
	if (!g_h)
	{
		if (relockMode)
			restart_vendor_service_if_found();
		MessageBoxW(NULL,
			L"The keyboard's RGB interface was found but could not be opened.\n"
			L"Another program may be holding it exclusively. Check rgb_engine_log.txt next to the exe for details.\n\n"
			L"Klavyenin RGB arayuzu bulundu ancak acilamadi.\n"
			L"Baska bir program onu paylasimsiz tutuyor olabilir. Detaylar icin exe'nin yanindaki rgb_engine_log.txt dosyasina bakin.",
			L"Better RGB - Error / Hata", MB_OK | MB_ICONERROR);
		return (1);
	}
	if (enter_custom_mode() < 0)
	{
		if (relockMode)
			restart_vendor_service_if_found();
		MessageBoxW(NULL,
			L"Could not enter per-key mode!\n\nPer-key moduna girilemedi!",
			L"Better RGB - Error / Hata", MB_OK | MB_ICONERROR);
		hid_close(g_h);
		hid_exit();
		return (1);
	}
	excl = log_exclusive_status();
	if (relockMode)
	{
		attempt = 0;
		while (!excl && attempt < 5)
		{
			attempt++;
			Sleep((DWORD)attempt * 1000);
			EnterCriticalSection(&g_deviceLock);
			if (g_h)
				hid_close(g_h);
			g_h = hid_open_path(g_devicePath);
			LeaveCriticalSection(&g_deviceLock);
			if (g_h && enter_custom_mode() == 0)
			{
				excl = log_exclusive_status();
				if (excl)
				{
					sprintf(lb, "relock: exclusive acquired on retry %d", attempt);
					log_event(lb);
				}
			}
		}
		restart_vendor_service_if_found();
		if (!g_h)
		{
			MessageBoxW(NULL,
				L"The keyboard device could not be reopened after the relock attempt.\n\n"
				L"Kilit yenileme denemesinden sonra klavye cihazi yeniden acilamadi.",
				L"Better RGB - Error / Hata", MB_OK | MB_ICONERROR);
			hid_exit();
			DeleteCriticalSection(&g_deviceLock);
			return (1);
		}
		if (!excl)
		{
			sprintf(lb, "relock: still SHARED after %d retries, refusing to start in shared mode", attempt);
			log_event(lb);
			MessageBoxW(NULL,
				L"The device lock could not be acquired: the vendor service is still holding the keyboard's RGB interface.\n"
				L"The application will not start in shared mode. Please try launching it again.\n\n"
				L"Cihaz kilidi alinamadi: uretici servisi klavyenin RGB arayuzunu hala elinde tutuyor.\n"
				L"Uygulama paylasimli modda baslatilmayacak. Lutfen uygulamayi tekrar baslatmayi deneyin.",
				L"Better RGB - Device Lock / Cihaz Kilidi", MB_OK | MB_ICONERROR);
			hid_close(g_h);
			hid_exit();
			DeleteCriticalSection(&g_deviceLock);
			return (1);
		}
	}
	else if (!excl)
	{
		if (is_elevated())
		{
			if (begin_relock() == 0)
			{
				hid_exit();
				DeleteCriticalSection(&g_deviceLock);
				return (0);
			}
		}
		else
		{
			ans = MessageBoxW(NULL,
				L"The vendor service (GCUBridge / Control Center) is currently holding the keyboard's RGB interface, so it can override this app's lighting at any moment.\n\n"
				L"Fix it now? The service will be restarted for a few seconds (fan / power functions are not affected) and the app will relaunch with the device locked. Windows will ask for administrator approval.\n\n"
				L"WARNING: choosing No means this app and the vendor service will write to the keyboard controller at the same time. Beyond your lighting being overridden, this conflict can LOCK UP the keyboard's embedded controller (EC).\n\n"
				L"--------------------\n\n"
				L"Uretici servisi (GCUBridge / Kontrol Merkezi) su anda klavyenin RGB arayuzunu elinde tutuyor; bu nedenle bu uygulamanin aydinlatma ayarlarinin uzerine istediginde yazabilir.\n\n"
				L"Simdi duzeltilsin mi? Servis birkac saniyeligine yeniden baslatilacak (fan / guc islevleri etkilenmez) ve uygulama cihaz kilitli sekilde yeniden acilacak. Windows yonetici onayi isteyecek.\n\n"
				L"UYARI: Hayir'i secerseniz bu uygulama ile uretici servisi klavye kontrolcusune ayni anda yazacak. Aydinlatma ayarlarinizin uzerine yazilmasinin otesinde, bu cakisma klavyenin gomulu kontrolcusunun (EC) KILITLENMESIYLE sonuclanabilir.",
				L"WARNING / UYARI - Better RGB Device Lock / Cihaz Kilidi", MB_YESNO | MB_ICONWARNING);
			if (ans == IDYES)
			{
				if (relaunch_elevated(NULL, 0) == 0)
				{
					hid_close(g_h);
					hid_exit();
					DeleteCriticalSection(&g_deviceLock);
					return (0);
				}
				log_event("elevation cancelled or failed, exiting without the lock");
				MessageBoxW(NULL,
					L"Device lock failed: the application was not granted administrator rights (the elevation prompt was cancelled or failed).\n"
					L"The application will now close. Please start it again and approve the administrator prompt.\n\n"
					L"--------------------\n\n"
					L"Cihaz kilidi basarisiz: uygulamaya yonetici haklari verilmedi (yukseltme istemi iptal edildi veya basarisiz oldu).\n"
					L"Uygulama simdi kapanacak. Lutfen uygulamayi tekrar baslatin ve yonetici onayini verin.",
					L"WARNING / UYARI - Better RGB", MB_OK | MB_ICONERROR);
				hid_close(g_h);
				hid_exit();
				DeleteCriticalSection(&g_deviceLock);
				return (1);
			}
			if (ans == IDNO)
			{
				ans2 = MessageBoxW(NULL,
					L"Are you sure you want to continue in shared mode?\n\n"
					L"Running alongside the vendor service can LOCK UP the keyboard's embedded controller (EC). If that happens, the keyboard lighting stops responding to ALL software, and recovery requires a full power-off reset: shut down, unplug the charger, and hold the power button for about 30 seconds.\n\n"
					L"Yes = continue in shared mode at my own risk\n"
					L"No = exit the application\n\n"
					L"--------------------\n\n"
					L"Paylasimli modda devam etmek istediginizden emin misiniz?\n\n"
					L"Uretici servisiyle ayni anda calismak, klavyenin gomulu kontrolcusunun (EC) KILITLENMESIYLE sonuclanabilir. Bu olursa klavye aydinlatmasi HICBIR yazilima yanit vermez ve kurtarmak icin tam guc sifirlamasi gerekir: bilgisayari kapatin, sarj kablosunu cikarin ve guc dugmesine yaklasik 30 saniye basili tutun.\n\n"
					L"Evet = riski kabul ederek paylasimli modda devam et\n"
					L"Hayir = uygulamadan cik",
					L"WARNING / UYARI", MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2);
				if (ans2 != IDYES)
				{
					hid_close(g_h);
					hid_exit();
					DeleteCriticalSection(&g_deviceLock);
					return (0);
				}
				log_event("user accepted shared mode despite EC lockup warning");
			}
		}
	}
	icc.dwSize = sizeof(icc);
	icc.dwICC = ICC_BAR_CLASSES;
	InitCommonControlsEx(&icc);
	memset(&swc, 0, sizeof(swc));
	swc.lpfnWndProc = SwatchProc;
	swc.hInstance = hInst;
	swc.lpszClassName = L"SwatchClass";
	swc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	RegisterClass(&swc);
	memset(&wc, 0, sizeof(wc));
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInst;
	wc.lpszClassName = L"RGBMainWindowClass";
	g_bgBrush = CreateSolidBrush(CLR_BG);
	wc.hbrBackground = g_bgBrush;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_APPICON));
	if (!RegisterClass(&wc))
	{
		MessageBox(NULL, L"RegisterClass failed!", L"Error", MB_OK | MB_ICONERROR);
		hid_close(g_h);
		hid_exit();
		return (1);
	}
	hwnd = CreateWindow(wc.lpszClassName, L"Better RGB by TheYamo",
			WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
			CW_USEDEFAULT, CW_USEDEFAULT, 400, 605,
			NULL, NULL, hInst, NULL);
	if (!hwnd)
	{
		MessageBox(NULL, L"CreateWindow failed!", L"Error", MB_OK | MB_ICONERROR);
		hid_close(g_h);
		hid_exit();
		return (1);
	}
	if (g_startupLaunch)
		log_event("startup launch: starting hidden in the system tray");
	else
	{
		ShowWindow(hwnd, SW_SHOW);
		UpdateWindow(hwnd);
	}
	if (g_wheelReverse)
		CheckDlgButton(hwnd, IDC_CHK_WHEEL_REVERSE, BST_CHECKED);
	if (g_reactiveDuration == 10)
		durationRadioId = IDC_RADIO_SHORT;
	else if (g_reactiveDuration == 45)
		durationRadioId = IDC_RADIO_LONG;
	else
		durationRadioId = IDC_RADIO_MEDIUM;
	CheckRadioButton(hwnd, IDC_RADIO_SHORT, IDC_RADIO_LONG, durationRadioId);
	SendMessage(g_hComboMatrixStyle, CB_SETCURSEL, (WPARAM)g_matrixStyle, 0);
	if (g_cfgMode >= 0 && g_cfgMode < MODE_COUNT)
	{
		SendMessage(g_hComboMode, CB_SETCURSEL, (WPARAM)g_cfgMode, 0);
		show_panel_for_mode((int)g_cfgMode);
		if (g_cfgMode == MODE_RAIN)
			refresh_rain_color_visibility();
		if (g_cfgMode == MODE_MATRIX)
			refresh_matrix_color_visibility();
		if (g_cfgMode == MODE_STATIC)
			refresh_static_ui();
		start_mode((int)g_cfgMode);
	}
	hPowerNotify = RegisterSuspendResumeNotification((HANDLE)hwnd, DEVICE_NOTIFY_WINDOW_HANDLE);
	if (!hPowerNotify)
		log_event("RegisterSuspendResumeNotification FAILED");
	else
		log_event("RegisterSuspendResumeNotification succeeded");
	memset(&devFilter, 0, sizeof(devFilter));
	devFilter.dbcc_size = sizeof(devFilter);
	devFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
	devFilter.dbcc_classguid = GUID_DEVINTERFACE_HID_LOCAL;
	hDevNotify = RegisterDeviceNotification(hwnd, &devFilter, DEVICE_NOTIFY_WINDOW_HANDLE);
	if (!hDevNotify)
		log_event("RegisterDeviceNotification FAILED");
	else
		log_event("RegisterDeviceNotification succeeded");
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	if (hPowerNotify)
		UnregisterSuspendResumeNotification(hPowerNotify);
	hid_close(g_h);
	hid_exit();
	DeleteCriticalSection(&g_deviceLock);
	return (0);
}
