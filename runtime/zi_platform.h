#pragma once

#include "zi_common.h"

ZI_HANDLER(ZiWindow);

enum ZiWindowFlags_
{
  ZiWindowFlags_None        = 0,
  ZiWindowFlags_Maximized   = 1 << 0,
};

typedef u32 ZiWindowFlags;

void zi_platform_init();
void zi_platform_terminate();

ZiWindow  zi_platform_create_window(const char* title, i32 width, i32 height, ZiWindowFlags flags);
void      zi_platform_destroy_window(ZiWindow window);
void      zi_platform_poll_events();
i8        zi_platform_should_close(ZiWindow window);

void zi_platform_console_log(const char* message, i32 len, u8 error);