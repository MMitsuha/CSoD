CSoD:Colorful Screen of Death

****************************************************
1.使用
	1.使用前请加载好驱动！
	2.本驱动会创建一个名为CSoD的设备，请使用CreateFile打开之以通信（独占设备，同时只能打开一次）
	（HANDLE hDevice = CreateFileW(L"\\\\.\\CSoD", GENERIC_ALL, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);）
	3.然后，请创建一个“CSoD_DATA”（在ControlCode.h中）类型的变量，并初始化之（成员解释请看注释）
	（
	CSoD_DATA Data = { 0 };
	Data.BackColor = BV_COLOR_BLACK;
	Data.TextColor = BV_COLOR_WHITE;
	）
	4.使用DeviceIoControl传入这个变量，并使用CTL_DO_CSoD控制码
	（
	BOOL bRet = DeviceIoControl(hDevice, CTL_DO_CSoD, &Data, sizeof(Data), NULL, 0, &dwReturned, NULL);
	）
*****************************************************
2.实例
#include <iostream>
#include <Windows.h>
#include "ControlCode.h"
using namespace std;

INT
wmain(
	IN PWCHAR wargv[],
	IN UINT wargc
)
{
	HANDLE hDevice = CreateFileW(L"\\\\.\\CSoD", GENERIC_ALL, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hDevice != INVALID_HANDLE_VALUE)
	{
		CSoD_DATA Data = { 0 };
		Data.BackColor = BV_COLOR_COLORFUL;
		Data.TextColor = BV_COLOR_COLORFUL;
		strcpy_s(Data.Text, "Hello World!");

		DWORD dwReturned = 0;
		BOOL bRet = DeviceIoControl(hDevice, CTL_DO_CSoD, &Data, sizeof(Data), NULL, 0, &dwReturned, NULL);
		if (bRet)
			;
	}

	return GetLastError();
}