#pragma once

#include "zi_common.h"

typedef struct ZiAppSettings {
	const char* title;
} ZiAppSettings;

u8 zi_app_is_running();