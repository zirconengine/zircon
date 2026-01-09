#pragma once

#include "zi_common.h"

// ============================================================================
// Allocator
// ============================================================================

typedef VoidPtr (*ZiAllocFn)(u64 size, VoidPtr user_data);
typedef void    (*ZiFreeFn)(VoidPtr ptr, VoidPtr user_data);

typedef struct ZiAllocator {
    ZiAllocFn alloc;
    ZiFreeFn  free;
    VoidPtr   user_data;
} ZiAllocator;

ZI_API ZiAllocator* zi_get_default_allocator(void);

// ============================================================================
// Hashmap
// ============================================================================

#define ZI_HASHMAP_INITIAL_CAPACITY 16
#define ZI_HASHMAP_LOAD_FACTOR 0.75f

#define ZI_HASHMAP(name, key_type, value_type)                                 \
                                                                               \
typedef struct name##_Entry {                                                  \
    key_type   key;                                                            \
    value_type value;                                                          \
    u8         occupied;                                                       \
    u8         deleted;                                                        \
} name##_Entry;                                                                \
                                                                               \
typedef struct name {                                                          \
    name##_Entry* entries;                                                     \
    u64           capacity;                                                    \
    u64           count;                                                       \
    ZiAllocator*  allocator;                                                   \
} name;                                                                        \
                                                                               \
static inline void name##_init(name* map, ZiAllocator* allocator) {            \
    map->allocator = allocator ? allocator : zi_get_default_allocator();       \
    map->capacity = ZI_HASHMAP_INITIAL_CAPACITY;                               \
    map->count = 0;                                                            \
    u64 size = sizeof(name##_Entry) * map->capacity;                           \
    map->entries = (name##_Entry*)map->allocator->alloc(size,                  \
                                                map->allocator->user_data);    \
    for (u64 i = 0; i < map->capacity; i++) {                                  \
        map->entries[i].occupied = 0;                                          \
        map->entries[i].deleted = 0;                                           \
    }                                                                          \
}                                                                              \
                                                                               \
static inline void name##_free(name* map) {                                    \
    if (map->entries) {                                                        \
        map->allocator->free(map->entries, map->allocator->user_data);         \
        map->entries = 0;                                                      \
    }                                                                          \
    map->capacity = 0;                                                         \
    map->count = 0;                                                            \
}                                                                              \
                                                                               \
static inline void name##_rehash(name* map, u64 new_capacity) {                \
    name##_Entry* old_entries = map->entries;                                  \
    u64 old_capacity = map->capacity;                                          \
                                                                               \
    u64 size = sizeof(name##_Entry) * new_capacity;                            \
    map->entries = (name##_Entry*)map->allocator->alloc(size,                  \
                                                map->allocator->user_data);    \
    map->capacity = new_capacity;                                              \
    map->count = 0;                                                            \
                                                                               \
    for (u64 i = 0; i < new_capacity; i++) {                                   \
        map->entries[i].occupied = 0;                                          \
        map->entries[i].deleted = 0;                                           \
    }                                                                          \
                                                                               \
    for (u64 i = 0; i < old_capacity; i++) {                                   \
        if (old_entries[i].occupied && !old_entries[i].deleted) {              \
            u64 hash = hash_##key_type(old_entries[i].key);                    \
            u64 idx = hash % map->capacity;                                    \
            while (map->entries[idx].occupied) {                               \
                idx = (idx + 1) % map->capacity;                               \
            }                                                                  \
            map->entries[idx].key = old_entries[i].key;                        \
            map->entries[idx].value = old_entries[i].value;                    \
            map->entries[idx].occupied = 1;                                    \
            map->entries[idx].deleted = 0;                                     \
            map->count++;                                                      \
        }                                                                      \
    }                                                                          \
                                                                               \
    map->allocator->free(old_entries, map->allocator->user_data);              \
}                                                                              \
                                                                               \
static inline void name##_resize(name* map) {                                  \
    name##_rehash(map, map->capacity * 2);                                     \
}                                                                              \
                                                                               \
static inline void name##_set(name* map, key_type key, value_type value) {     \
    if ((f32)(map->count + 1) > (f32)map->capacity * ZI_HASHMAP_LOAD_FACTOR) { \
        name##_resize(map);                                                    \
    }                                                                          \
                                                                               \
    u64 hash = hash_##key_type(key);                                           \
    u64 idx = hash % map->capacity;                                            \
    u64 first_deleted = U64_MAX;                                               \
                                                                               \
    while (map->entries[idx].occupied) {                                       \
        if (map->entries[idx].deleted) {                                       \
            if (first_deleted == U64_MAX) {                                    \
                first_deleted = idx;                                           \
            }                                                                  \
        } else if (compare_##key_type(map->entries[idx].key, key)) {           \
            map->entries[idx].value = value;                                   \
            return;                                                            \
        }                                                                      \
        idx = (idx + 1) % map->capacity;                                       \
    }                                                                          \
                                                                               \
    if (first_deleted != U64_MAX) {                                            \
        idx = first_deleted;                                                   \
    }                                                                          \
                                                                               \
    map->entries[idx].key = key;                                               \
    map->entries[idx].value = value;                                           \
    map->entries[idx].occupied = 1;                                            \
    map->entries[idx].deleted = 0;                                             \
    map->count++;                                                              \
}                                                                              \
                                                                               \
static inline value_type* name##_get(name* map, key_type key) {                \
    u64 hash = hash_##key_type(key);                                           \
    u64 idx = hash % map->capacity;                                            \
    u64 start_idx = idx;                                                       \
                                                                               \
    while (map->entries[idx].occupied) {                                       \
        if (!map->entries[idx].deleted &&                                      \
            compare_##key_type(map->entries[idx].key, key)) {                  \
            return &map->entries[idx].value;                                   \
        }                                                                      \
        idx = (idx + 1) % map->capacity;                                       \
        if (idx == start_idx) break;                                           \
    }                                                                          \
    return 0;                                                                  \
}                                                                              \
                                                                               \
static inline i8 name##_has(name* map, key_type key) {                         \
    return name##_get(map, key) != 0;                                          \
}                                                                              \
                                                                               \
static inline i8 name##_remove(name* map, key_type key) {                      \
    u64 hash = hash_##key_type(key);                                           \
    u64 idx = hash % map->capacity;                                            \
    u64 start_idx = idx;                                                       \
                                                                               \
    while (map->entries[idx].occupied) {                                       \
        if (!map->entries[idx].deleted &&                                      \
            compare_##key_type(map->entries[idx].key, key)) {                  \
            map->entries[idx].deleted = 1;                                     \
            map->count--;                                                      \
            return 1;                                                          \
        }                                                                      \
        idx = (idx + 1) % map->capacity;                                       \
        if (idx == start_idx) break;                                           \
    }                                                                          \
    return 0;                                                                  \
}                                                                              \
                                                                               \
static inline void name##_clear(name* map) {                                   \
    for (u64 i = 0; i < map->capacity; i++) {                                  \
        map->entries[i].occupied = 0;                                          \
        map->entries[i].deleted = 0;                                           \
    }                                                                          \
    map->count = 0;                                                            \
}

