#pragma once

#include "zi_common.h"

enum ZiLogLevel_ {
	ZiLogLevel_Trace    = 0,
	ZiLogLevel_Debug    = 1,
	ZiLogLevel_Info     = 2,
	ZiLogLevel_Warn     = 3,
	ZiLogLevel_Error    = 4,
	ZiLogLevel_Critical = 5,
	ZiLogLevel_Off      = 6
};

typedef u8 ZiLogLevel;

void zi_log(ZiLogLevel level, const char* fmt, ...);

#define zi_log_debug(...) zi_log(ZiLogLevel_Debug, __VA_ARGS__)
#define zi_log_info(...) zi_log(ZiLogLevel_Info, __VA_ARGS__)
#define zi_log_error(...) zi_log(ZiLogLevel_Error, __VA_ARGS__)
