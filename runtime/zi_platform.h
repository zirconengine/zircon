#pragma once

#include "zi_common.h"
#include "zi_graphics.h"


// Time
f64               zi_platform_get_time(void);
void              zi_platform_console_log(const char* message, i32 len, u8 error);
i32               zi_platform_get_timestamp(char* buf, i32 buf_size);
ZiGraphicsBackend zi_platform_get_graphics_backend(ZiGraphicsBackend backend);
