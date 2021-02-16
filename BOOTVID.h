#pragma once
#include <ntifs.h>

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

#define VGA_CHAR_SIZE 2

#define TEXT_CHAR_SIZE 2

#define CHAR_WIDTH  8
#define CHAR_HEIGHT 16

#define SCREEN_WIDTH  640
#define SCREEN_HEIGHT 400
#define BYTES_PER_SCANLINE (SCREEN_WIDTH / 8)

typedef enum _INBV_DISPLAY_STATE
{
	INBV_DISPLAY_STATE_OWNED,     // we own the display
	INBV_DISPLAY_STATE_DISABLED,  // we own but should not use
	INBV_DISPLAY_STATE_LOST       // we lost ownership
} INBV_DISPLAY_STATE;

typedef
BOOLEAN
(*INBV_RESET_DISPLAY_PARAMETERS)(
	ULONG Cols,
	ULONG Rows
	);

typedef
VOID
(*INBV_DISPLAY_STRING_FILTER)(
	PUCHAR* Str
	);

NTKERNELAPI
VOID
InbvNotifyDisplayOwnershipLost(
	INBV_RESET_DISPLAY_PARAMETERS ResetDisplayParameters
);

NTKERNELAPI
VOID
InbvInstallDisplayStringFilter(
	INBV_DISPLAY_STRING_FILTER DisplayStringFilter
);

NTKERNELAPI
VOID
InbvAcquireDisplayOwnership(
	VOID
);

BOOLEAN
InbvDriverInitialize(
	IN PVOID LoaderBlock,
	IN ULONG Count
);

NTKERNELAPI
BOOLEAN
InbvResetDisplay(
);

VOID
InbvBitBlt(
	PUCHAR Buffer,
	ULONG x,
	ULONG y
);

NTKERNELAPI
VOID
InbvSolidColorFill(
	ULONG x1,
	ULONG y1,
	ULONG x2,
	ULONG y2,
	ULONG color
);

NTKERNELAPI
BOOLEAN
InbvDisplayString(
	PUCHAR Str
);

VOID
InbvUpdateProgressBar(
	ULONG Percentage
);

VOID
InbvSetProgressBarSubset(
	ULONG   Floor,
	ULONG   Ceiling
);

VOID
InbvSetBootDriverBehavior(
	PVOID LoaderBlock
);

VOID
InbvIndicateProgress(
	VOID
);

VOID
InbvSetProgressBarCoordinates(
	ULONG x,
	ULONG y
);

NTKERNELAPI
VOID
InbvEnableBootDriver(
	BOOLEAN bEnable
);

NTKERNELAPI
BOOLEAN
InbvEnableDisplayString(
	BOOLEAN bEnable
);

NTKERNELAPI
BOOLEAN
InbvIsBootDriverInstalled(
	VOID
);

PUCHAR
InbvGetResourceAddress(
	IN ULONG ResourceNumber
);

VOID
InbvBufferToScreenBlt(
	PUCHAR Buffer,
	ULONG x,
	ULONG y,
	ULONG width,
	ULONG height,
	ULONG lDelta
);

VOID
InbvScreenToBufferBlt(
	PUCHAR Buffer,
	ULONG x,
	ULONG y,
	ULONG width,
	ULONG height,
	ULONG lDelta
);

BOOLEAN
InbvTestLock(
	VOID
);

VOID
InbvAcquireLock(
	VOID
);

VOID
InbvReleaseLock(
	VOID
);

NTKERNELAPI
BOOLEAN
InbvCheckDisplayOwnership(
	VOID
);

NTKERNELAPI
VOID
InbvSetScrollRegion(
	ULONG x1,
	ULONG y1,
	ULONG x2,
	ULONG y2
);

NTKERNELAPI
ULONG
InbvSetTextColor(
	ULONG Color
);