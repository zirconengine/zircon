#include "zi_core.h"
#include <stdlib.h>

static VoidPtr zi_default_alloc(u64 size, VoidPtr user_data) {
    (void)user_data;
    return malloc(size);
}

static void zi_default_free(VoidPtr ptr, VoidPtr user_data) {
    (void)user_data;
    free(ptr);
}

static ZiAllocator g_default_allocator = {
    .alloc     = zi_default_alloc,
    .free      = zi_default_free,
    .user_data = 0
};

ZiAllocator* zi_get_default_allocator(void) {
    return &g_default_allocator;
}
