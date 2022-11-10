#include <Windows.h>
#include <locale.h>
#include <stdio.h>
#include <strsafe.h>
#include<tchar.h>
#include<string>
#include"printsys.h"


void ShowErrorExit() {
	WCHAR lpMsgBuf[256] = { 0, };
	WCHAR lpShowBuf[512] = { 0, };
	DWORD dw = GetLastError();

	FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT), lpMsgBuf,
		sizeof(lpMsgBuf), NULL);	//格式化错误消息

	StringCchPrintf(lpShowBuf, sizeof(lpShowBuf), L"GetLastError:0x%x,FormatMessage:%s", dw, lpMsgBuf);

	wprintf(lpShowBuf);  //使用wprintf会出现乱码，需要加入setlocale(LC_CTYPE, "");
	ExitProcess(dw);
}

int InstallDriver(IN SC_HANDLE SchSCManager, IN LPCTSTR DriverName, IN LPCTSTR ServiceExe)
{
	SC_HANDLE  schService;
	int retvalue = 0;

	// NOTE: This creates an entry for a standalone driver. If this
	//       is modified for use with a driver that requires a Tag,
	//       Group, and/or Dependencies, it may be necessary to
	//       query the registry for existing driver information
	//       (in order to determine a unique Tag, etc.).
	//

	schService = CreateService(SchSCManager,          // SCManager database
		DriverName,           // name of service
		DriverName,           // name to display
		SERVICE_ALL_ACCESS,    // desired access
		SERVICE_KERNEL_DRIVER, // service type
		SERVICE_DEMAND_START,  // start type
		SERVICE_ERROR_NORMAL,  // error control type
		ServiceExe,            // service's binary
		NULL,                  // no load ordering group
		NULL,                  // no tag identifier
		NULL,                  // no dependencies
		NULL,                  // LocalSystem account
		NULL                   // no password
	);
	if (schService == NULL)
	{
		retvalue = GetLastError();
		//if (ERROR_SERVICE_EXISTS == retvalue) retvalue = 0;
	}
	else
		CloseServiceHandle(schService);
	return retvalue;
}

int StartDriver(IN SC_HANDLE SchSCManager, IN LPCTSTR DriverName)
{
	SC_HANDLE  schService;
	int ret = 0;

	schService = OpenService(SchSCManager,
		DriverName,
		SERVICE_ALL_ACCESS
	);
	if (schService == NULL)
	{
		ret = GetLastError();
		return ret;
	}

	if ((ret = StartService(schService, 0, NULL)) == 0)
	{
		ret = GetLastError();	//ERROR_SERVICE_ALREADY_RUNNING
	}
	else
		ret = 0;

	CloseServiceHandle(schService);
	return ret;
}

BOOL StopDriver(IN SC_HANDLE SchSCManager, IN LPCTSTR DriverName)
{
	SC_HANDLE       schService;
	BOOL            ret;
	SERVICE_STATUS  serviceStatus;

	schService = OpenService(SchSCManager, DriverName, SERVICE_ALL_ACCESS);
	if (schService == NULL)
		return FALSE;

	ret = ControlService(schService, SERVICE_CONTROL_STOP, &serviceStatus);
	CloseServiceHandle(schService);
	return ret;
}

BOOL RemoveDriver(IN SC_HANDLE SchSCManager, IN LPCTSTR DriverName)
{
	SC_HANDLE  schService;
	BOOL       ret;

	schService = OpenService(SchSCManager,
		DriverName,
		SERVICE_ALL_ACCESS
	);

	if (schService == NULL)
		return FALSE;

	ret = DeleteService(schService);
	SERVICE_STATUS  serviceStatus;
	ret = QueryServiceStatus(schService, &serviceStatus);
	CloseServiceHandle(schService);
	return ret;
}

//非0为失败,0为成功
int LoadDeviceDriver(const TCHAR* Name, const TCHAR* Path)
{
	OVERLAPPED  overlap = { 0 };
	SC_HANDLE   schSCManager;
	int         retval = 0;

	schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	//忽略重复安装失败
	retval = InstallDriver(schSCManager, Name, Path);
	retval = StartDriver(schSCManager, Name);
	CloseServiceHandle(schSCManager);
	return retval;
}

//False为失败
BOOL UnloadDeviceDriver(const TCHAR* Name)
{
	SC_HANDLE   schSCManager;
	BOOL ret;

	schSCManager = OpenSCManager(NULL,                 // machine (NULL == local)
		NULL,                 // database (NULL == default)
		SC_MANAGER_ALL_ACCESS // access required
	);

	//忽略重复停止失败
	ret = StopDriver(schSCManager, Name);
	ret = RemoveDriver(schSCManager, Name);
	CloseServiceHandle(schSCManager);
	return ret;
}

bool GetFileName(wchar_t* path, _Out_ wchar_t* filename) {
	std::wstring str(path);
	auto npos1 = str.find_last_of(L"\\");
	if (-1 == npos1) npos1 = 0;

	auto npos2 = str.find_last_of(L".");
	if (-1 == npos2 || npos1 >= npos2) npos2 = str.length();

	std::wstring name = str.substr(npos1 + 1, npos2 - (npos1 + 1));
	memcpy_s(filename, MAX_PATH, name.data(), name.length() * 2);
	return TRUE;
}

int _tmain(int args, wchar_t* argv[]) {
	setlocale(LC_CTYPE, "");

	__try {
		wchar_t* option = argv[1];
		wchar_t path[MAX_PATH] = { 0 };
		wchar_t serviceName[MAX_PATH] = { 0 };

		if (wcscmp(option, L"-i") == 0) {
			if (!GetFullPathNameW(argv[2], MAX_PATH, path, NULL)) ShowErrorExit();
			GetFileName(path, serviceName);

			if (0 != LoadDeviceDriver(serviceName, path)) {
				ShowErrorExit();
			}
			else {
				wprintf(L"LoadDeviceDriver %ws is success.\n", serviceName);
				return 0;
			}
		}

		if (wcscmp(option, L"-u") == 0) {
			if (!GetFullPathNameW(argv[2], MAX_PATH, path, NULL)) ShowErrorExit();
			GetFileName(path, serviceName);

			if (!UnloadDeviceDriver(serviceName)) {
				ShowErrorExit();
			}
			else {
				wprintf(L"UnloadDeviceDriver %ws is success.\n", serviceName);
				return 0;
			};
		}

		if (wcscmp(option, L"-c") == 0) {
			PrintSysCompany();
			return 0;
		}

		if (wcscmp(option, L"-q") == 0) {
			PrintExternSys();
			return 0;
		}

		RaiseException(1, 0, 0, NULL);
	}
	__except (1) {
		wprintf(L"参数错误，请参照以下指令:\n");
		wprintf(L"\t安装sys驱动:		InstallSys.exe -i [syspath/service]\n");
		wprintf(L"\t卸载sys驱动:		InstallSys.exe -u [syspath/service]\n");
		wprintf(L"\t查询非系统驱动:		InstallSys.exe -q\n");
		wprintf(L"\t查询所有模块和公司:	InstallSys.exe -c\n");
	}
	return 0;
}
