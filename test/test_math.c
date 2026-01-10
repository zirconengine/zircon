#include "unity.h"
#include "zi_math.h"

// ============================================================================
// Utility Functions Tests
// ============================================================================

void test_zi_min_f32(void) {
    TEST_ASSERT_FLOAT_WITHIN(ZI_EPSILON, 1.0f, zi_min_f32(1.0f, 2.0f));
    TEST_ASSERT_FLOAT_WITHIN(ZI_EPSILON, -5.0f, zi_min_f32(-5.0f, 3.0f));
    TEST_ASSERT_FLOAT_WITHIN(ZI_EPSILON, 0.0f, zi_min_f32(0.0f, 0.0f));
}

void test_zi_max_f32(void) {
    TEST_ASSERT_FLOAT_WITHIN(ZI_EPSILON, 2.0f, zi_max_f32(1.0f, 2.0f));
    TEST_ASSERT_FLOAT_WITHIN(ZI_EPSILON, 3.0f, zi_max_f32(-5.0f, 3.0f));
    TEST_ASSERT_FLOAT_WITHIN(ZI_EPSILON, 0.0f, zi_max_f32(0.0f, 0.0f));
}

void test_zi_clamp_f32(void) {
    TEST_ASSERT_FLOAT_WITHIN(ZI_EPSILON, 0.5f, zi_clamp_f32(0.5f, 0.0f, 1.0f));
    TEST_ASSERT_FLOAT_WITHIN(ZI_EPSILON, 0.0f, zi_clamp_f32(-1.0f, 0.0f, 1.0f));
    TEST_ASSERT_FLOAT_WITHIN(ZI_EPSILON, 1.0f, zi_clamp_f32(5.0f, 0.0f, 1.0f));
}

void test_zi_lerp_f32(void) {
    TEST_ASSERT_FLOAT_WITHIN(ZI_EPSILON, 0.0f, zi_lerp_f32(0.0f, 10.0f, 0.0f));
    TEST_ASSERT_FLOAT_WITHIN(ZI_EPSILON, 5.0f, zi_lerp_f32(0.0f, 10.0f, 0.5f));
    TEST_ASSERT_FLOAT_WITHIN(ZI_EPSILON, 10.0f, zi_lerp_f32(0.0f, 10.0f, 1.0f));
}

void test_zi_abs_f32(void) {
    TEST_ASSERT_FLOAT_WITHIN(ZI_EPSILON, 5.0f, zi_abs_f32(5.0f));
    TEST_ASSERT_FLOAT_WITHIN(ZI_EPSILON, 5.0f, zi_abs_f32(-5.0f));
    TEST_ASSERT_FLOAT_WITHIN(ZI_EPSILON, 0.0f, zi_abs_f32(0.0f));
}

void test_zi_radians_degrees(void) {
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, ZI_PI, zi_radians(180.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, ZI_HALF_PI, zi_radians(90.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 180.0f, zi_degrees(ZI_PI));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 90.0f, zi_degrees(ZI_HALF_PI));
}

// ============================================================================
// Integer Utility Tests
// ============================================================================

void test_zi_min_i32(void) {
    TEST_ASSERT_EQUAL_INT32(1, zi_min_i32(1, 2));
    TEST_ASSERT_EQUAL_INT32(-5, zi_min_i32(-5, 3));
}

void test_zi_max_i32(void) {
    TEST_ASSERT_EQUAL_INT32(2, zi_max_i32(1, 2));
    TEST_ASSERT_EQUAL_INT32(3, zi_max_i32(-5, 3));
}

void test_zi_clamp_i32(void) {
    TEST_ASSERT_EQUAL_INT32(5, zi_clamp_i32(5, 0, 10));
    TEST_ASSERT_EQUAL_INT32(0, zi_clamp_i32(-5, 0, 10));
    TEST_ASSERT_EQUAL_INT32(10, zi_clamp_i32(15, 0, 10));
}

// ============================================================================
// Smoothstep Tests
// ============================================================================

void test_zi_smoothstep(void) {
    TEST_ASSERT_FLOAT_WITHIN(ZI_EPSILON, 0.0f, zi_smoothstep(0.0f, 1.0f, 0.0f));
    TEST_ASSERT_FLOAT_WITHIN(ZI_EPSILON, 1.0f, zi_smoothstep(0.0f, 1.0f, 1.0f));
    TEST_ASSERT_FLOAT_WITHIN(ZI_EPSILON, 0.5f, zi_smoothstep(0.0f, 1.0f, 0.5f));
}

void test_zi_inverse_lerp(void) {
    TEST_ASSERT_FLOAT_WITHIN(ZI_EPSILON, 0.5f, zi_inverse_lerp(0.0f, 10.0f, 5.0f));
    TEST_ASSERT_FLOAT_WITHIN(ZI_EPSILON, 0.0f, zi_inverse_lerp(0.0f, 10.0f, 0.0f));
    TEST_ASSERT_FLOAT_WITHIN(ZI_EPSILON, 1.0f, zi_inverse_lerp(0.0f, 10.0f, 10.0f));
}

void test_zi_sign_f32(void) {
    TEST_ASSERT_FLOAT_WITHIN(ZI_EPSILON, 1.0f, zi_sign_f32(5.0f));
    TEST_ASSERT_FLOAT_WITHIN(ZI_EPSILON, -1.0f, zi_sign_f32(-5.0f));
    TEST_ASSERT_FLOAT_WITHIN(ZI_EPSILON, 0.0f, zi_sign_f32(0.0f));
}

// ============================================================================
// Vector2 Tests
// ============================================================================

void test_zi_vec2_add(void) {
    ZiVec2 a = zi_vec2(1.0f, 2.0f);
    ZiVec2 b = zi_vec2(3.0f, 4.0f);
    ZiVec2 result = zi_vec2_add(a, b);

    TEST_ASSERT_FLOAT_WITHIN(ZI_EPSILON, 4.0f, result.x);
    TEST_ASSERT_FLOAT_WITHIN(ZI_EPSILON, 6.0f, result.y);
}

void test_zi_vec2_sub(void) {
    ZiVec2 a = zi_vec2(5.0f, 7.0f);
    ZiVec2 b = zi_vec2(2.0f, 3.0f);
    ZiVec2 result = zi_vec2_sub(a, b);

    TEST_ASSERT_FLOAT_WITHIN(ZI_EPSILON, 3.0f, result.x);
    TEST_ASSERT_FLOAT_WITHIN(ZI_EPSILON, 4.0f, result.y);
}

void test_zi_vec2_scale(void) {
    ZiVec2 v = zi_vec2(2.0f, 3.0f);
    ZiVec2 result = zi_vec2_scale(v, 2.0f);

    TEST_ASSERT_FLOAT_WITHIN(ZI_EPSILON, 4.0f, result.x);
    TEST_ASSERT_FLOAT_WITHIN(ZI_EPSILON, 6.0f, result.y);
}

void test_zi_vec2_dot(void) {
    ZiVec2 a = zi_vec2(1.0f, 2.0f);
    ZiVec2 b = zi_vec2(3.0f, 4.0f);

    TEST_ASSERT_FLOAT_WITHIN(ZI_EPSILON, 11.0f, zi_vec2_dot(a, b));
}

void test_zi_vec2_length(void) {
    ZiVec2 v = zi_vec2(3.0f, 4.0f);
    TEST_ASSERT_FLOAT_WITHIN(ZI_EPSILON, 5.0f, zi_vec2_length(v));
}

void test_zi_vec2_normalize(void) {
    ZiVec2 v = zi_vec2(3.0f, 4.0f);
    ZiVec2 result = zi_vec2_normalize(v);

    TEST_ASSERT_FLOAT_WITHIN(ZI_EPSILON, 0.6f, result.x);
    TEST_ASSERT_FLOAT_WITHIN(ZI_EPSILON, 0.8f, result.y);
    TEST_ASSERT_FLOAT_WITHIN(ZI_EPSILON, 1.0f, zi_vec2_length(result));
}

void test_zi_vec2_normalize_zero(void) {
    ZiVec2 v = zi_vec2_zero();
    ZiVec2 result = zi_vec2_normalize(v);

    TEST_ASSERT_FLOAT_WITHIN(ZI_EPSILON, 0.0f, result.x);
    TEST_ASSERT_FLOAT_WITHIN(ZI_EPSILON, 0.0f, result.y);
}

// ============================================================================
// Test Runner
// ============================================================================

void run_math_tests(void) {
    // Utility functions
    RUN_TEST(test_zi_min_f32);
    RUN_TEST(test_zi_max_f32);
    RUN_TEST(test_zi_clamp_f32);
    RUN_TEST(test_zi_lerp_f32);
    RUN_TEST(test_zi_abs_f32);
    RUN_TEST(test_zi_radians_degrees);

    // Integer utilities
    RUN_TEST(test_zi_min_i32);
    RUN_TEST(test_zi_max_i32);
    RUN_TEST(test_zi_clamp_i32);

    // Smoothstep and related
    RUN_TEST(test_zi_smoothstep);
    RUN_TEST(test_zi_inverse_lerp);
    RUN_TEST(test_zi_sign_f32);

    // Vector2
    RUN_TEST(test_zi_vec2_add);
    RUN_TEST(test_zi_vec2_sub);
    RUN_TEST(test_zi_vec2_scale);
    RUN_TEST(test_zi_vec2_dot);
    RUN_TEST(test_zi_vec2_length);
    RUN_TEST(test_zi_vec2_normalize);
    RUN_TEST(test_zi_vec2_normalize_zero);
}
