#include <stdlib.h>

#include "unity.h"
#include "zi_core.h"
#include <string.h>

// ============================================================================
// Test Allocator Tracking (for verifying allocations)
// ============================================================================

static u64 g_alloc_count = 0;
static u64 g_free_count = 0;
static u64 g_total_allocated = 0;

static VoidPtr test_alloc(u64 size, VoidPtr user_data) {
    (void)user_data;
    g_alloc_count++;
    g_total_allocated += size;
    return malloc(size);
}

static void test_free(VoidPtr ptr, VoidPtr user_data) {
    (void)user_data;
    if (ptr) {
        g_free_count++;
        free(ptr);
    }
}

static ZiAllocator g_test_allocator = {
    .alloc     = test_alloc,
    .free      = test_free,
    .user_data = 0
};

static void reset_alloc_stats(void) {
    g_alloc_count = 0;
    g_free_count = 0;
    g_total_allocated = 0;
}

// ============================================================================
// Hash and Compare Functions for Hashmap Tests
// ============================================================================

// i32 key type
static inline u64 hash_i32(i32 key) {
    return (u64)key * 2654435761u;
}

static inline i8 compare_i32(i32 a, i32 b) {
    return a == b;
}

// u64 key type
static inline u64 hash_u64(u64 key) {
    key ^= key >> 33;
    key *= 0xff51afd7ed558ccdULL;
    key ^= key >> 33;
    key *= 0xc4ceb9fe1a85ec53ULL;
    key ^= key >> 33;
    return key;
}

static inline i8 compare_u64(u64 a, u64 b) {
    return a == b;
}

// String key type (ConstChr)
static inline u64 hash_ConstChr(ConstChr key) {
    u64 hash = 5381;
    i32 c;
    while ((c = *key++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

static inline i8 compare_ConstChr(ConstChr a, ConstChr b) {
    return strcmp(a, b) == 0;
}

// ============================================================================
// Hashmap Type Declarations
// ============================================================================

ZI_HASHMAP(IntMap, i32, i32);
ZI_HASHMAP(U64Map, u64, f32);
ZI_HASHMAP(StringMap, ConstChr, i32);

// ============================================================================
// Array Type Declarations
// ============================================================================

ZI_ARRAY(IntArray, i32);
ZI_ARRAY(F32Array, f32);
ZI_ARRAY(PtrArray, VoidPtr);

typedef struct TestStruct {
    i32 x;
    i32 y;
    f32 z;
} TestStruct;

ZI_ARRAY(StructArray, TestStruct);

// ============================================================================
// Core Test Setup/Teardown
// ============================================================================

static void core_test_setup(void) {
    reset_alloc_stats();
}

// ============================================================================
// Allocator Tests
// ============================================================================

void test_default_allocator_exists(void) {
    ZiAllocator* alloc = zi_get_default_allocator();
    TEST_ASSERT_NOT_NULL(alloc);
    TEST_ASSERT_NOT_NULL(alloc->alloc);
    TEST_ASSERT_NOT_NULL(alloc->free);
}

void test_default_allocator_alloc_free(void) {
    ZiAllocator* alloc = zi_get_default_allocator();

    VoidPtr ptr = alloc->alloc(128, alloc->user_data);
    TEST_ASSERT_NOT_NULL(ptr);

    // Write to memory to ensure it's valid
    memset(ptr, 0xAB, 128);

    alloc->free(ptr, alloc->user_data);
}

void test_custom_allocator_tracking(void) {
    VoidPtr ptr1 = g_test_allocator.alloc(64, g_test_allocator.user_data);
    VoidPtr ptr2 = g_test_allocator.alloc(128, g_test_allocator.user_data);

    TEST_ASSERT_EQUAL_UINT64(2, g_alloc_count);
    TEST_ASSERT_EQUAL_UINT64(192, g_total_allocated);
    TEST_ASSERT_EQUAL_UINT64(0, g_free_count);

    g_test_allocator.free(ptr1, g_test_allocator.user_data);
    g_test_allocator.free(ptr2, g_test_allocator.user_data);

    TEST_ASSERT_EQUAL_UINT64(2, g_free_count);
}

// ============================================================================
// Hashmap Tests - Integer Keys
// ============================================================================

void test_intmap_init_free(void) {
    IntMap map;
    IntMap_init(&map, &g_test_allocator);

    TEST_ASSERT_NOT_NULL(map.entries);
    TEST_ASSERT_EQUAL_UINT64(ZI_HASHMAP_INITIAL_CAPACITY, map.capacity);
    TEST_ASSERT_EQUAL_UINT64(0, map.count);
    TEST_ASSERT_EQUAL_UINT64(1, g_alloc_count);

    IntMap_free(&map);
    TEST_ASSERT_EQUAL_UINT64(1, g_free_count);
}

void test_intmap_set_get(void) {
    IntMap map;
    IntMap_init(&map, &g_test_allocator);

    IntMap_set(&map, 42, 100);
    IntMap_set(&map, 13, 200);
    IntMap_set(&map, 99, 300);

    TEST_ASSERT_EQUAL_UINT64(3, map.count);

    i32* val1 = IntMap_get(&map, 42);
    i32* val2 = IntMap_get(&map, 13);
    i32* val3 = IntMap_get(&map, 99);
    i32* val_missing = IntMap_get(&map, 999);

    TEST_ASSERT_NOT_NULL(val1);
    TEST_ASSERT_NOT_NULL(val2);
    TEST_ASSERT_NOT_NULL(val3);
    TEST_ASSERT_NULL(val_missing);

    TEST_ASSERT_EQUAL_INT32(100, *val1);
    TEST_ASSERT_EQUAL_INT32(200, *val2);
    TEST_ASSERT_EQUAL_INT32(300, *val3);

    IntMap_free(&map);
}

void test_intmap_overwrite(void) {
    IntMap map;
    IntMap_init(&map, &g_test_allocator);

    IntMap_set(&map, 42, 100);
    TEST_ASSERT_EQUAL_INT32(100, *IntMap_get(&map, 42));

    IntMap_set(&map, 42, 999);
    TEST_ASSERT_EQUAL_INT32(999, *IntMap_get(&map, 42));
    TEST_ASSERT_EQUAL_UINT64(1, map.count);

    IntMap_free(&map);
}

void test_intmap_has(void) {
    IntMap map;
    IntMap_init(&map, &g_test_allocator);

    TEST_ASSERT_FALSE(IntMap_has(&map, 42));

    IntMap_set(&map, 42, 100);
    TEST_ASSERT_TRUE(IntMap_has(&map, 42));
    TEST_ASSERT_FALSE(IntMap_has(&map, 999));

    IntMap_free(&map);
}

void test_intmap_remove(void) {
    IntMap map;
    IntMap_init(&map, &g_test_allocator);

    IntMap_set(&map, 42, 100);
    IntMap_set(&map, 13, 200);
    TEST_ASSERT_EQUAL_UINT64(2, map.count);

    i8 removed = IntMap_remove(&map, 42);
    TEST_ASSERT_TRUE(removed);
    TEST_ASSERT_EQUAL_UINT64(1, map.count);
    TEST_ASSERT_FALSE(IntMap_has(&map, 42));
    TEST_ASSERT_TRUE(IntMap_has(&map, 13));

    // Try removing non-existent key
    removed = IntMap_remove(&map, 999);
    TEST_ASSERT_FALSE(removed);

    IntMap_free(&map);
}

void test_intmap_clear(void) {
    IntMap map;
    IntMap_init(&map, &g_test_allocator);

    IntMap_set(&map, 1, 10);
    IntMap_set(&map, 2, 20);
    IntMap_set(&map, 3, 30);
    TEST_ASSERT_EQUAL_UINT64(3, map.count);

    IntMap_clear(&map);
    TEST_ASSERT_EQUAL_UINT64(0, map.count);
    TEST_ASSERT_FALSE(IntMap_has(&map, 1));
    TEST_ASSERT_FALSE(IntMap_has(&map, 2));
    TEST_ASSERT_FALSE(IntMap_has(&map, 3));

    IntMap_free(&map);
}

void test_intmap_resize(void) {
    IntMap map;
    IntMap_init(&map, &g_test_allocator);

    u64 initial_capacity = map.capacity;

    // Insert enough items to trigger resize
    for (i32 i = 0; i < 20; i++) {
        IntMap_set(&map, i, i * 10);
    }

    TEST_ASSERT_GREATER_THAN_UINT64(initial_capacity, map.capacity);
    TEST_ASSERT_EQUAL_UINT64(20, map.count);

    // Verify all values are still correct after resize
    for (i32 i = 0; i < 20; i++) {
        i32* val = IntMap_get(&map, i);
        TEST_ASSERT_NOT_NULL(val);
        TEST_ASSERT_EQUAL_INT32(i * 10, *val);
    }

    IntMap_free(&map);
}

// ============================================================================
// Hashmap Tests - u64 Keys with f32 Values
// ============================================================================

void test_u64map_basic(void) {
    U64Map map;
    U64Map_init(&map, &g_test_allocator);

    U64Map_set(&map, 1000000000ULL, 1.5f);
    U64Map_set(&map, 2000000000ULL, 2.5f);
    U64Map_set(&map, U64_MAX, 3.5f);

    f32* val1 = U64Map_get(&map, 1000000000ULL);
    f32* val2 = U64Map_get(&map, 2000000000ULL);
    f32* val3 = U64Map_get(&map, U64_MAX);

    TEST_ASSERT_NOT_NULL(val1);
    TEST_ASSERT_NOT_NULL(val2);
    TEST_ASSERT_NOT_NULL(val3);

    TEST_ASSERT_FLOAT_WITHIN(0.001f, 1.5f, *val1);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 2.5f, *val2);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 3.5f, *val3);

    U64Map_free(&map);
}

// ============================================================================
// Hashmap Tests - String Keys
// ============================================================================

void test_stringmap_basic(void) {
    StringMap map;
    StringMap_init(&map, &g_test_allocator);

    StringMap_set(&map, "hello", 1);
    StringMap_set(&map, "world", 2);
    StringMap_set(&map, "zircon", 3);

    TEST_ASSERT_EQUAL_UINT64(3, map.count);

    i32* val1 = StringMap_get(&map, "hello");
    i32* val2 = StringMap_get(&map, "world");
    i32* val3 = StringMap_get(&map, "zircon");
    i32* val_missing = StringMap_get(&map, "missing");

    TEST_ASSERT_NOT_NULL(val1);
    TEST_ASSERT_NOT_NULL(val2);
    TEST_ASSERT_NOT_NULL(val3);
    TEST_ASSERT_NULL(val_missing);

    TEST_ASSERT_EQUAL_INT32(1, *val1);
    TEST_ASSERT_EQUAL_INT32(2, *val2);
    TEST_ASSERT_EQUAL_INT32(3, *val3);

    StringMap_free(&map);
}

void test_stringmap_collision_handling(void) {
    StringMap map;
    StringMap_init(&map, &g_test_allocator);

    // Insert many strings to test collision handling
    StringMap_set(&map, "a", 1);
    StringMap_set(&map, "b", 2);
    StringMap_set(&map, "c", 3);
    StringMap_set(&map, "aa", 4);
    StringMap_set(&map, "bb", 5);
    StringMap_set(&map, "cc", 6);
    StringMap_set(&map, "aaa", 7);
    StringMap_set(&map, "bbb", 8);

    TEST_ASSERT_EQUAL_INT32(1, *StringMap_get(&map, "a"));
    TEST_ASSERT_EQUAL_INT32(2, *StringMap_get(&map, "b"));
    TEST_ASSERT_EQUAL_INT32(7, *StringMap_get(&map, "aaa"));
    TEST_ASSERT_EQUAL_INT32(8, *StringMap_get(&map, "bbb"));

    StringMap_free(&map);
}

// ============================================================================
// Array Tests - Integer Array
// ============================================================================

void test_intarray_init_free(void) {
    IntArray arr;
    IntArray_init(&arr, &g_test_allocator);

    TEST_ASSERT_NOT_NULL(arr.data);
    TEST_ASSERT_EQUAL_UINT64(ZI_ARRAY_INITIAL_CAPACITY, arr.capacity);
    TEST_ASSERT_EQUAL_UINT64(0, arr.count);
    TEST_ASSERT_EQUAL_UINT64(1, g_alloc_count);

    IntArray_free(&arr);
    TEST_ASSERT_EQUAL_UINT64(1, g_free_count);
}

void test_intarray_init_capacity(void) {
    IntArray arr;
    IntArray_init_capacity(&arr, &g_test_allocator, 100);

    TEST_ASSERT_EQUAL_UINT64(100, arr.capacity);
    TEST_ASSERT_EQUAL_UINT64(0, arr.count);

    IntArray_free(&arr);
}

void test_intarray_push_pop(void) {
    IntArray arr;
    IntArray_init(&arr, &g_test_allocator);

    IntArray_push(&arr, 10);
    IntArray_push(&arr, 20);
    IntArray_push(&arr, 30);

    TEST_ASSERT_EQUAL_UINT64(3, arr.count);

    TEST_ASSERT_EQUAL_INT32(30, IntArray_pop(&arr));
    TEST_ASSERT_EQUAL_INT32(20, IntArray_pop(&arr));
    TEST_ASSERT_EQUAL_INT32(10, IntArray_pop(&arr));

    TEST_ASSERT_EQUAL_UINT64(0, arr.count);

    IntArray_free(&arr);
}

void test_intarray_get_set(void) {
    IntArray arr;
    IntArray_init(&arr, &g_test_allocator);

    IntArray_push(&arr, 100);
    IntArray_push(&arr, 200);
    IntArray_push(&arr, 300);

    TEST_ASSERT_EQUAL_INT32(100, *IntArray_get(&arr, 0));
    TEST_ASSERT_EQUAL_INT32(200, *IntArray_get(&arr, 1));
    TEST_ASSERT_EQUAL_INT32(300, *IntArray_get(&arr, 2));
    TEST_ASSERT_NULL(IntArray_get(&arr, 3));

    IntArray_set(&arr, 1, 999);
    TEST_ASSERT_EQUAL_INT32(999, *IntArray_get(&arr, 1));

    IntArray_free(&arr);
}

void test_intarray_first_last(void) {
    IntArray arr;
    IntArray_init(&arr, &g_test_allocator);

    TEST_ASSERT_NULL(IntArray_first(&arr));
    TEST_ASSERT_NULL(IntArray_last(&arr));

    IntArray_push(&arr, 10);
    TEST_ASSERT_EQUAL_INT32(10, *IntArray_first(&arr));
    TEST_ASSERT_EQUAL_INT32(10, *IntArray_last(&arr));

    IntArray_push(&arr, 20);
    IntArray_push(&arr, 30);
    TEST_ASSERT_EQUAL_INT32(10, *IntArray_first(&arr));
    TEST_ASSERT_EQUAL_INT32(30, *IntArray_last(&arr));

    IntArray_free(&arr);
}

void test_intarray_insert(void) {
    IntArray arr;
    IntArray_init(&arr, &g_test_allocator);

    IntArray_push(&arr, 10);
    IntArray_push(&arr, 30);

    // Insert in the middle
    IntArray_insert(&arr, 1, 20);

    TEST_ASSERT_EQUAL_UINT64(3, arr.count);
    TEST_ASSERT_EQUAL_INT32(10, *IntArray_get(&arr, 0));
    TEST_ASSERT_EQUAL_INT32(20, *IntArray_get(&arr, 1));
    TEST_ASSERT_EQUAL_INT32(30, *IntArray_get(&arr, 2));

    // Insert at beginning
    IntArray_insert(&arr, 0, 5);
    TEST_ASSERT_EQUAL_INT32(5, *IntArray_get(&arr, 0));
    TEST_ASSERT_EQUAL_INT32(10, *IntArray_get(&arr, 1));

    IntArray_free(&arr);
}

void test_intarray_remove(void) {
    IntArray arr;
    IntArray_init(&arr, &g_test_allocator);

    IntArray_push(&arr, 10);
    IntArray_push(&arr, 20);
    IntArray_push(&arr, 30);
    IntArray_push(&arr, 40);

    // Remove from middle (preserves order)
    IntArray_remove(&arr, 1);

    TEST_ASSERT_EQUAL_UINT64(3, arr.count);
    TEST_ASSERT_EQUAL_INT32(10, *IntArray_get(&arr, 0));
    TEST_ASSERT_EQUAL_INT32(30, *IntArray_get(&arr, 1));
    TEST_ASSERT_EQUAL_INT32(40, *IntArray_get(&arr, 2));

    IntArray_free(&arr);
}

void test_intarray_remove_swap(void) {
    IntArray arr;
    IntArray_init(&arr, &g_test_allocator);

    IntArray_push(&arr, 10);
    IntArray_push(&arr, 20);
    IntArray_push(&arr, 30);
    IntArray_push(&arr, 40);

    // Remove with swap (faster, doesn't preserve order)
    IntArray_remove_swap(&arr, 1);

    TEST_ASSERT_EQUAL_UINT64(3, arr.count);
    TEST_ASSERT_EQUAL_INT32(10, *IntArray_get(&arr, 0));
    TEST_ASSERT_EQUAL_INT32(40, *IntArray_get(&arr, 1)); // 40 swapped to index 1
    TEST_ASSERT_EQUAL_INT32(30, *IntArray_get(&arr, 2));

    IntArray_free(&arr);
}

void test_intarray_clear(void) {
    IntArray arr;
    IntArray_init(&arr, &g_test_allocator);

    IntArray_push(&arr, 10);
    IntArray_push(&arr, 20);
    IntArray_push(&arr, 30);

    IntArray_clear(&arr);
    TEST_ASSERT_EQUAL_UINT64(0, arr.count);
    TEST_ASSERT_GREATER_THAN_UINT64(0, arr.capacity); // Capacity preserved

    IntArray_free(&arr);
}

void test_intarray_reserve(void) {
    IntArray arr;
    IntArray_init(&arr, &g_test_allocator);

    IntArray_push(&arr, 10);
    IntArray_push(&arr, 20);

    IntArray_reserve(&arr, 100);

    TEST_ASSERT_EQUAL_UINT64(100, arr.capacity);
    TEST_ASSERT_EQUAL_UINT64(2, arr.count);
    TEST_ASSERT_EQUAL_INT32(10, *IntArray_get(&arr, 0));
    TEST_ASSERT_EQUAL_INT32(20, *IntArray_get(&arr, 1));

    IntArray_free(&arr);
}

void test_intarray_grow(void) {
    IntArray arr;
    IntArray_init(&arr, &g_test_allocator);

    u64 initial_capacity = arr.capacity;

    // Push more than initial capacity to trigger grow
    for (i32 i = 0; i < 20; i++) {
        IntArray_push(&arr, i);
    }

    TEST_ASSERT_GREATER_THAN_UINT64(initial_capacity, arr.capacity);
    TEST_ASSERT_EQUAL_UINT64(20, arr.count);

    // Verify values
    for (i32 i = 0; i < 20; i++) {
        TEST_ASSERT_EQUAL_INT32(i, *IntArray_get(&arr, i));
    }

    IntArray_free(&arr);
}

// ============================================================================
// Array Tests - Float Array
// ============================================================================

void test_f32array_basic(void) {
    F32Array arr;
    F32Array_init(&arr, &g_test_allocator);

    F32Array_push(&arr, 1.5f);
    F32Array_push(&arr, 2.5f);
    F32Array_push(&arr, 3.5f);

    TEST_ASSERT_FLOAT_WITHIN(0.001f, 1.5f, *F32Array_get(&arr, 0));
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 2.5f, *F32Array_get(&arr, 1));
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 3.5f, *F32Array_get(&arr, 2));

    F32Array_free(&arr);
}

// ============================================================================
// Array Tests - Struct Array
// ============================================================================

void test_structarray_basic(void) {
    StructArray arr;
    StructArray_init(&arr, &g_test_allocator);

    TestStruct s1 = { .x = 1, .y = 2, .z = 3.0f };
    TestStruct s2 = { .x = 4, .y = 5, .z = 6.0f };
    TestStruct s3 = { .x = 7, .y = 8, .z = 9.0f };

    StructArray_push(&arr, s1);
    StructArray_push(&arr, s2);
    StructArray_push(&arr, s3);

    TEST_ASSERT_EQUAL_UINT64(3, arr.count);

    TestStruct* p1 = StructArray_get(&arr, 0);
    TestStruct* p2 = StructArray_get(&arr, 1);
    TestStruct* p3 = StructArray_get(&arr, 2);

    TEST_ASSERT_EQUAL_INT32(1, p1->x);
    TEST_ASSERT_EQUAL_INT32(2, p1->y);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 3.0f, p1->z);

    TEST_ASSERT_EQUAL_INT32(4, p2->x);
    TEST_ASSERT_EQUAL_INT32(5, p2->y);

    TEST_ASSERT_EQUAL_INT32(7, p3->x);
    TEST_ASSERT_EQUAL_INT32(8, p3->y);

    StructArray_free(&arr);
}

// ============================================================================
// Test Runner
// ============================================================================

void run_core_tests(void) {
    // Allocator tests
    core_test_setup();
    RUN_TEST(test_default_allocator_exists);
    core_test_setup();
    RUN_TEST(test_default_allocator_alloc_free);
    core_test_setup();
    RUN_TEST(test_custom_allocator_tracking);

    // IntMap (i32 -> i32) tests
    core_test_setup();
    RUN_TEST(test_intmap_init_free);
    core_test_setup();
    RUN_TEST(test_intmap_set_get);
    core_test_setup();
    RUN_TEST(test_intmap_overwrite);
    core_test_setup();
    RUN_TEST(test_intmap_has);
    core_test_setup();
    RUN_TEST(test_intmap_remove);
    core_test_setup();
    RUN_TEST(test_intmap_clear);
    core_test_setup();
    RUN_TEST(test_intmap_resize);

    // U64Map (u64 -> f32) tests
    core_test_setup();
    RUN_TEST(test_u64map_basic);

    // StringMap (ConstChr -> i32) tests
    core_test_setup();
    RUN_TEST(test_stringmap_basic);
    core_test_setup();
    RUN_TEST(test_stringmap_collision_handling);

    // IntArray tests
    core_test_setup();
    RUN_TEST(test_intarray_init_free);
    core_test_setup();
    RUN_TEST(test_intarray_init_capacity);
    core_test_setup();
    RUN_TEST(test_intarray_push_pop);
    core_test_setup();
    RUN_TEST(test_intarray_get_set);
    core_test_setup();
    RUN_TEST(test_intarray_first_last);
    core_test_setup();
    RUN_TEST(test_intarray_insert);
    core_test_setup();
    RUN_TEST(test_intarray_remove);
    core_test_setup();
    RUN_TEST(test_intarray_remove_swap);
    core_test_setup();
    RUN_TEST(test_intarray_clear);
    core_test_setup();
    RUN_TEST(test_intarray_reserve);
    core_test_setup();
    RUN_TEST(test_intarray_grow);

    // F32Array tests
    core_test_setup();
    RUN_TEST(test_f32array_basic);

    // StructArray tests
    core_test_setup();
    RUN_TEST(test_structarray_basic);
}
