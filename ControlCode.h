#pragma once

#ifndef CTL_CODE
#define CTL_CODE( DeviceType, Function, Method, Access ) (                 \
    ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) \
)
#endif

#ifndef METHOD_BUFFERED
#define METHOD_BUFFERED                 0
#endif

#ifndef FILE_DEVICE_UNKNOWN
#define FILE_DEVICE_UNKNOWN             0x00000022
#endif

#ifndef FILE_ANY_ACCESS
#define FILE_ANY_ACCESS                 0
#endif

#ifndef FILE_SPECIAL_ACCESS
#define FILE_SPECIAL_ACCESS    (FILE_ANY_ACCESS)
#endif

#ifndef FILE_READ_ACCESS
#define FILE_READ_ACCESS          ( 0x0001 )    // file & pipe
#endif

#ifndef FILE_WRITE_ACCESS
#define FILE_WRITE_ACCESS         ( 0x0002 )    // file & pipe
#endif

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

#define SCREEN_WIDTH  640
#define SCREEN_HEIGHT 400

#define CHAR_WIDTH  8
#define CHAR_HEIGHT 16

#define MAKECODE(Function,Access) (CTL_CODE(FILE_DEVICE_UNKNOWN,Function,METHOD_BUFFERED,Access))

#define CTL_DO_CSoD MAKECODE(0x1C05,FILE_WRITE_ACCESS)

typedef struct _CSoD_DATA
{
	UCHAR BackColor;
	UCHAR TextColor;

	CHAR Text[(SCREEN_WIDTH / CHAR_WIDTH) * (SCREEN_HEIGHT / CHAR_HEIGHT)];
}CSoD_DATA, * PCSoD_DATA;