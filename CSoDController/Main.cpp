#include "Includes.hpp"

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