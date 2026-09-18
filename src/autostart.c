#include <windows.h>
#include <wchar.h>
#include "autostart.h"

# define TASK_NAME	L"Better RGB by TheYamo"

HWND	g_btnAutostart = NULL;

static int	run_schtasks(const wchar_t *args)
{
	wchar_t				cmd_line[1200];
	STARTUPINFOW		si;
	PROCESS_INFORMATION	pi;
	DWORD				code;

	_snwprintf(cmd_line, 1199, L"schtasks.exe %ls", args);
	cmd_line[1199] = 0;
	memset(&si, 0, sizeof(si));
	memset(&pi, 0, sizeof(pi));
	si.cb = sizeof(si);
	code = 1;
	if (CreateProcessW(NULL, cmd_line, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
	{
		WaitForSingleObject(pi.hProcess, 20000);
		GetExitCodeProcess(pi.hProcess, &code);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
	return ((int)code);
}

int	autostart_task_exists(void)
{
	return (run_schtasks(L"/Query /TN \"" TASK_NAME L"\"") == 0);
}

static int	autostart_install(void)
{
	wchar_t			exe[MAX_PATH];
	wchar_t			user[128];
	wchar_t			domain[128];
	wchar_t			account[300];
	wchar_t			temp_dir[MAX_PATH];
	wchar_t			xml_path[MAX_PATH];
	wchar_t			xml[4096];
	wchar_t			args[MAX_PATH + 128];
	HANDLE			f;
	unsigned short	bom;
	DWORD			written;
	int				r;

	user[0] = 0;
	domain[0] = 0;
	GetModuleFileNameW(NULL, exe, MAX_PATH);
	GetEnvironmentVariableW(L"USERDOMAIN", domain, 128);
	GetEnvironmentVariableW(L"USERNAME", user, 128);
	if (!user[0])
		return (-1);
	if (domain[0])
		_snwprintf(account, 299, L"%ls\\%ls", domain, user);
	else
		_snwprintf(account, 299, L"%ls", user);
	account[299] = 0;
	if (!GetTempPathW(MAX_PATH, temp_dir))
		return (-1);
	_snwprintf(xml_path, MAX_PATH - 1, L"%lsbetter_rgb_task.xml", temp_dir);
	xml_path[MAX_PATH - 1] = 0;
	_snwprintf(xml, 4095,
		L"<?xml version=\"1.0\" encoding=\"UTF-16\"?>\r\n"
		L"<Task version=\"1.2\" xmlns=\"http://schemas.microsoft.com/windows/2004/02/mit/task\">\r\n"
		L"  <Triggers>\r\n"
		L"    <LogonTrigger>\r\n"
		L"      <Enabled>true</Enabled>\r\n"
		L"      <UserId>%ls</UserId>\r\n"
		L"    </LogonTrigger>\r\n"
		L"  </Triggers>\r\n"
		L"  <Principals>\r\n"
		L"    <Principal id=\"Author\">\r\n"
		L"      <UserId>%ls</UserId>\r\n"
		L"      <LogonType>InteractiveToken</LogonType>\r\n"
		L"      <RunLevel>HighestAvailable</RunLevel>\r\n"
		L"    </Principal>\r\n"
		L"  </Principals>\r\n"
		L"  <Settings>\r\n"
		L"    <MultipleInstancesPolicy>IgnoreNew</MultipleInstancesPolicy>\r\n"
		L"    <DisallowStartIfOnBatteries>false</DisallowStartIfOnBatteries>\r\n"
		L"    <StopIfGoingOnBatteries>false</StopIfGoingOnBatteries>\r\n"
		L"    <AllowHardTerminate>false</AllowHardTerminate>\r\n"
		L"    <StartWhenAvailable>false</StartWhenAvailable>\r\n"
		L"    <AllowStartOnDemand>true</AllowStartOnDemand>\r\n"
		L"    <Enabled>true</Enabled>\r\n"
		L"    <Hidden>false</Hidden>\r\n"
		L"    <ExecutionTimeLimit>PT0S</ExecutionTimeLimit>\r\n"
		L"    <Priority>7</Priority>\r\n"
		L"  </Settings>\r\n"
		L"  <Actions Context=\"Author\">\r\n"
		L"    <Exec>\r\n"
		L"      <Command>\"%ls\"</Command>\r\n"
		L"      <Arguments>--startup</Arguments>\r\n"
		L"    </Exec>\r\n"
		L"  </Actions>\r\n"
		L"</Task>\r\n",
		account, account, exe);
	xml[4095] = 0;
	f = CreateFileW(xml_path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (f == INVALID_HANDLE_VALUE)
		return (-1);
	bom = 0xFEFF;
	written = 0;
	WriteFile(f, &bom, 2, &written, NULL);
	WriteFile(f, xml, (DWORD)(wcslen(xml) * sizeof(wchar_t)), &written, NULL);
	CloseHandle(f);
	_snwprintf(args, MAX_PATH + 127, L"/Create /F /TN \"" TASK_NAME L"\" /XML \"%ls\"", xml_path);
	args[MAX_PATH + 127] = 0;
	r = run_schtasks(args);
	DeleteFileW(xml_path);
	if (r == 0)
		return (0);
	return (-1);
}

static int	autostart_remove(void)
{
	if (run_schtasks(L"/Delete /F /TN \"" TASK_NAME L"\"") == 0)
		return (0);
	return (-1);
}

void	autostart_apply(int install)
{
	int				r;
	const wchar_t	*msg;
	UINT			icon;

	if (install)
		r = autostart_install();
	else
		r = autostart_remove();
	if (r == 0 && install)
		msg = L"Auto-start at login is now ENABLED.\n"
			L"The app will launch automatically with admin rights at every login, with no UAC prompt.\n\n"
			L"Oturum acilisinda otomatik baslatma ACILDI.\n"
			L"Uygulama her oturum acilisinda yonetici haklariyla, UAC sorusu olmadan kendiliginden baslayacak.";
	else if (r == 0)
		msg = L"Auto-start at login is now DISABLED.\n\n"
			L"Oturum acilisinda otomatik baslatma KAPATILDI.";
	else
		msg = L"The operation failed. Please try again.\n\n"
			L"Islem basarisiz oldu. Lutfen tekrar deneyin.";
	if (r == 0)
		icon = MB_ICONINFORMATION;
	else
		icon = MB_ICONERROR;
	MessageBoxW(NULL, msg, L"Better RGB by TheYamo", MB_OK | icon);
}

void	update_autostart_button(void)
{
	if (!g_btnAutostart)
		return ;
	if (autostart_task_exists())
		SetWindowTextW(g_btnAutostart, L"Auto-Start at Login: ON / Otomatik Baslatma: ACIK");
	else
		SetWindowTextW(g_btnAutostart, L"Auto-Start at Login: OFF / Otomatik Baslatma: KAPALI");
	InvalidateRect(g_btnAutostart, NULL, TRUE);
}
