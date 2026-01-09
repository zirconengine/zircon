#pragma once

#include "zi_common.h"
#include <math.h>

// ============================================================================
// Constants
// ============================================================================

#define ZI_PI          3.14159265358979323846f
#define ZI_TWO_PI      6.28318530717958647692f
#define ZI_HALF_PI     1.57079632679489661923f
#define ZI_INV_PI      0.31830988618379067154f
#define ZI_DEG_TO_RAD  0.01745329251994329577f
#define ZI_RAD_TO_DEG  57.2957795130823208768f
#define ZI_EPSILON     1e-6f

// ============================================================================
// Utility Macros / Inline Functions
// ============================================================================

static inline f32 zi_min_f32(f32 a, f32 b) { return a < b ? a : b; }
static inline f32 zi_max_f32(f32 a, f32 b) { return a > b ? a : b; }
static inline f32 zi_clamp_f32(f32 v, f32 lo, f32 hi) { return zi_min_f32(zi_max_f32(v, lo), hi); }
static inline f32 zi_lerp_f32(f32 a, f32 b, f32 t) { return a + (b - a) * t; }
static inline f32 zi_abs_f32(f32 v) { return v < 0.0f ? -v : v; }
static inline f32 zi_radians(f32 deg) { return deg * ZI_DEG_TO_RAD; }
static inline f32 zi_degrees(f32 rad) { return rad * ZI_RAD_TO_DEG; }

static inline i32 zi_min_i32(i32 a, i32 b) { return a < b ? a : b; }
static inline i32 zi_max_i32(i32 a, i32 b) { return a > b ? a : b; }
static inline i32 zi_clamp_i32(i32 v, i32 lo, i32 hi) { return zi_min_i32(zi_max_i32(v, lo), hi); }
static inline i32 zi_abs_i32(i32 v) { return v < 0 ? -v : v; }

// Linear to sRGB conversion
static inline f32 zi_linear_to_srgb(f32 linear) {
    if (linear <= 0.0031308f) {
        return linear * 12.92f;
    }
    return 1.055f * powf(linear, 1.0f / 2.4f) - 0.055f;
}

// sRGB to Linear conversion
static inline f32 zi_srgb_to_linear(f32 srgb) {
    if (srgb <= 0.04045f) {
        return srgb / 12.92f;
    }
    return powf((srgb + 0.055f) / 1.055f, 2.4f);
}

// ============================================================================
// Vector2 (f32)
// ============================================================================

typedef struct ZiVec2 {
    f32 x, y;
} ZiVec2;

static inline ZiVec2 zi_vec2(f32 x, f32 y) {
    return (ZiVec2){ x, y };
}

static inline ZiVec2 zi_vec2_zero(void) {
    return (ZiVec2){ 0.0f, 0.0f };
}

static inline ZiVec2 zi_vec2_one(void) {
    return (ZiVec2){ 1.0f, 1.0f };
}

static inline ZiVec2 zi_vec2_add(ZiVec2 a, ZiVec2 b) {
    return (ZiVec2){ a.x + b.x, a.y + b.y };
}

static inline ZiVec2 zi_vec2_sub(ZiVec2 a, ZiVec2 b) {
    return (ZiVec2){ a.x - b.x, a.y - b.y };
}

static inline ZiVec2 zi_vec2_mul(ZiVec2 a, ZiVec2 b) {
    return (ZiVec2){ a.x * b.x, a.y * b.y };
}

static inline ZiVec2 zi_vec2_div(ZiVec2 a, ZiVec2 b) {
    return (ZiVec2){ a.x / b.x, a.y / b.y };
}

static inline ZiVec2 zi_vec2_scale(ZiVec2 v, f32 s) {
    return (ZiVec2){ v.x * s, v.y * s };
}

static inline f32 zi_vec2_dot(ZiVec2 a, ZiVec2 b) {
    return a.x * b.x + a.y * b.y;
}

static inline f32 zi_vec2_length_sq(ZiVec2 v) {
    return zi_vec2_dot(v, v);
}

static inline f32 zi_vec2_length(ZiVec2 v) {
    return sqrtf(zi_vec2_length_sq(v));
}

static inline ZiVec2 zi_vec2_normalize(ZiVec2 v) {
    f32 len = zi_vec2_length(v);
    if (len > ZI_EPSILON) {
        return zi_vec2_scale(v, 1.0f / len);
    }
    return zi_vec2_zero();
}

static inline ZiVec2 zi_vec2_lerp(ZiVec2 a, ZiVec2 b, f32 t) {
    return (ZiVec2){ zi_lerp_f32(a.x, b.x, t), zi_lerp_f32(a.y, b.y, t) };
}

static inline ZiVec2 zi_vec2_negate(ZiVec2 v) {
    return (ZiVec2){ -v.x, -v.y };
}

// ============================================================================
// Vector2 (i32)
// ============================================================================

typedef struct ZiIVec2 {
    i32 x, y;
} ZiIVec2;

static inline ZiIVec2 zi_ivec2(i32 x, i32 y) {
    return (ZiIVec2){ x, y };
}

static inline ZiIVec2 zi_ivec2_add(ZiIVec2 a, ZiIVec2 b) {
    return (ZiIVec2){ a.x + b.x, a.y + b.y };
}

static inline ZiIVec2 zi_ivec2_sub(ZiIVec2 a, ZiIVec2 b) {
    return (ZiIVec2){ a.x - b.x, a.y - b.y };
}

static inline ZiIVec2 zi_ivec2_mul(ZiIVec2 a, ZiIVec2 b) {
    return (ZiIVec2){ a.x * b.x, a.y * b.y };
}

static inline ZiIVec2 zi_ivec2_scale(ZiIVec2 v, i32 s) {
    return (ZiIVec2){ v.x * s, v.y * s };
}

// ============================================================================
// Vector3 (f32)
// ============================================================================

typedef struct ZiVec3 {
    f32 x, y, z;
} ZiVec3;

static inline ZiVec3 zi_vec3(f32 x, f32 y, f32 z) {
    return (ZiVec3){ x, y, z };
}

static inline ZiVec3 zi_vec3_zero(void) {
    return (ZiVec3){ 0.0f, 0.0f, 0.0f };
}

static inline ZiVec3 zi_vec3_one(void) {
    return (ZiVec3){ 1.0f, 1.0f, 1.0f };
}

static inline ZiVec3 zi_vec3_up(void) {
    return (ZiVec3){ 0.0f, 1.0f, 0.0f };
}

static inline ZiVec3 zi_vec3_right(void) {
    return (ZiVec3){ 1.0f, 0.0f, 0.0f };
}

static inline ZiVec3 zi_vec3_forward(void) {
    return (ZiVec3){ 0.0f, 0.0f, 1.0f };
}

static inline ZiVec3 zi_vec3_add(ZiVec3 a, ZiVec3 b) {
    return (ZiVec3){ a.x + b.x, a.y + b.y, a.z + b.z };
}

static inline ZiVec3 zi_vec3_sub(ZiVec3 a, ZiVec3 b) {
    return (ZiVec3){ a.x - b.x, a.y - b.y, a.z - b.z };
}

static inline ZiVec3 zi_vec3_mul(ZiVec3 a, ZiVec3 b) {
    return (ZiVec3){ a.x * b.x, a.y * b.y, a.z * b.z };
}

static inline ZiVec3 zi_vec3_div(ZiVec3 a, ZiVec3 b) {
    return (ZiVec3){ a.x / b.x, a.y / b.y, a.z / b.z };
}

static inline ZiVec3 zi_vec3_scale(ZiVec3 v, f32 s) {
    return (ZiVec3){ v.x * s, v.y * s, v.z * s };
}

static inline f32 zi_vec3_dot(ZiVec3 a, ZiVec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static inline ZiVec3 zi_vec3_cross(ZiVec3 a, ZiVec3 b) {
    return (ZiVec3){
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

static inline f32 zi_vec3_length_sq(ZiVec3 v) {
    return zi_vec3_dot(v, v);
}

static inline f32 zi_vec3_length(ZiVec3 v) {
    return sqrtf(zi_vec3_length_sq(v));
}

static inline ZiVec3 zi_vec3_normalize(ZiVec3 v) {
    f32 len = zi_vec3_length(v);
    if (len > ZI_EPSILON) {
        return zi_vec3_scale(v, 1.0f / len);
    }
    return zi_vec3_zero();
}

static inline ZiVec3 zi_vec3_lerp(ZiVec3 a, ZiVec3 b, f32 t) {
    return (ZiVec3){
        zi_lerp_f32(a.x, b.x, t),
        zi_lerp_f32(a.y, b.y, t),
        zi_lerp_f32(a.z, b.z, t)
    };
}

static inline ZiVec3 zi_vec3_negate(ZiVec3 v) {
    return (ZiVec3){ -v.x, -v.y, -v.z };
}

static inline ZiVec3 zi_vec3_reflect(ZiVec3 v, ZiVec3 n) {
    f32 d = 2.0f * zi_vec3_dot(v, n);
    return zi_vec3_sub(v, zi_vec3_scale(n, d));
}

// ============================================================================
// Vector3 (i32)
// ============================================================================

typedef struct ZiIVec3 {
    i32 x, y, z;
} ZiIVec3;

static inline ZiIVec3 zi_ivec3(i32 x, i32 y, i32 z) {
    return (ZiIVec3){ x, y, z };
}

static inline ZiIVec3 zi_ivec3_add(ZiIVec3 a, ZiIVec3 b) {
    return (ZiIVec3){ a.x + b.x, a.y + b.y, a.z + b.z };
}

static inline ZiIVec3 zi_ivec3_sub(ZiIVec3 a, ZiIVec3 b) {
    return (ZiIVec3){ a.x - b.x, a.y - b.y, a.z - b.z };
}

static inline ZiIVec3 zi_ivec3_mul(ZiIVec3 a, ZiIVec3 b) {
    return (ZiIVec3){ a.x * b.x, a.y * b.y, a.z * b.z };
}

static inline ZiIVec3 zi_ivec3_scale(ZiIVec3 v, i32 s) {
    return (ZiIVec3){ v.x * s, v.y * s, v.z * s };
}

// ============================================================================
// Vector4 (f32)
// ============================================================================

typedef struct ZiVec4 {
    f32 x, y, z, w;
} ZiVec4;

static inline ZiVec4 zi_vec4(f32 x, f32 y, f32 z, f32 w) {
    return (ZiVec4){ x, y, z, w };
}

static inline ZiVec4 zi_vec4_zero(void) {
    return (ZiVec4){ 0.0f, 0.0f, 0.0f, 0.0f };
}

static inline ZiVec4 zi_vec4_one(void) {
    return (ZiVec4){ 1.0f, 1.0f, 1.0f, 1.0f };
}

static inline ZiVec4 zi_vec4_from_vec3(ZiVec3 v, f32 w) {
    return (ZiVec4){ v.x, v.y, v.z, w };
}

static inline ZiVec3 zi_vec4_to_vec3(ZiVec4 v) {
    return (ZiVec3){ v.x, v.y, v.z };
}

static inline ZiVec4 zi_vec4_add(ZiVec4 a, ZiVec4 b) {
    return (ZiVec4){ a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w };
}

static inline ZiVec4 zi_vec4_sub(ZiVec4 a, ZiVec4 b) {
    return (ZiVec4){ a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w };
}

static inline ZiVec4 zi_vec4_mul(ZiVec4 a, ZiVec4 b) {
    return (ZiVec4){ a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w };
}

static inline ZiVec4 zi_vec4_div(ZiVec4 a, ZiVec4 b) {
    return (ZiVec4){ a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w };
}

static inline ZiVec4 zi_vec4_scale(ZiVec4 v, f32 s) {
    return (ZiVec4){ v.x * s, v.y * s, v.z * s, v.w * s };
}

static inline f32 zi_vec4_dot(ZiVec4 a, ZiVec4 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

static inline f32 zi_vec4_length_sq(ZiVec4 v) {
    return zi_vec4_dot(v, v);
}

static inline f32 zi_vec4_length(ZiVec4 v) {
    return sqrtf(zi_vec4_length_sq(v));
}

static inline ZiVec4 zi_vec4_normalize(ZiVec4 v) {
    f32 len = zi_vec4_length(v);
    if (len > ZI_EPSILON) {
        return zi_vec4_scale(v, 1.0f / len);
    }
    return zi_vec4_zero();
}

static inline ZiVec4 zi_vec4_lerp(ZiVec4 a, ZiVec4 b, f32 t) {
    return (ZiVec4){
        zi_lerp_f32(a.x, b.x, t),
        zi_lerp_f32(a.y, b.y, t),
        zi_lerp_f32(a.z, b.z, t),
        zi_lerp_f32(a.w, b.w, t)
    };
}

// ============================================================================
// Vector4 (i32)
// ============================================================================

typedef struct ZiIVec4 {
    i32 x, y, z, w;
} ZiIVec4;

static inline ZiIVec4 zi_ivec4(i32 x, i32 y, i32 z, i32 w) {
    return (ZiIVec4){ x, y, z, w };
}

static inline ZiIVec4 zi_ivec4_add(ZiIVec4 a, ZiIVec4 b) {
    return (ZiIVec4){ a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w };
}

static inline ZiIVec4 zi_ivec4_sub(ZiIVec4 a, ZiIVec4 b) {
    return (ZiIVec4){ a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w };
}

static inline ZiIVec4 zi_ivec4_mul(ZiIVec4 a, ZiIVec4 b) {
    return (ZiIVec4){ a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w };
}

static inline ZiIVec4 zi_ivec4_scale(ZiIVec4 v, i32 s) {
    return (ZiIVec4){ v.x * s, v.y * s, v.z * s, v.w * s };
}

// ============================================================================
// Matrix 3x3 (column-major)
// ============================================================================

typedef struct ZiMat3 {
    f32 m[9]; // Column-major: m[col * 3 + row]
} ZiMat3;

static inline ZiMat3 zi_mat3_identity(void) {
    return (ZiMat3){{
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    }};
}

static inline ZiMat3 zi_mat3_zero(void) {
    return (ZiMat3){{ 0.0f }};
}

static inline f32 zi_mat3_at(const ZiMat3* m, i32 row, i32 col) {
    return m->m[col * 3 + row];
}

static inline void zi_mat3_set(ZiMat3* m, i32 row, i32 col, f32 val) {
    m->m[col * 3 + row] = val;
}

static inline ZiMat3 zi_mat3_mul(const ZiMat3* a, const ZiMat3* b) {
    ZiMat3 result = zi_mat3_zero();
    for (i32 col = 0; col < 3; col++) {
        for (i32 row = 0; row < 3; row++) {
            f32 sum = 0.0f;
            for (i32 k = 0; k < 3; k++) {
                sum += zi_mat3_at(a, row, k) * zi_mat3_at(b, k, col);
            }
            zi_mat3_set(&result, row, col, sum);
        }
    }
    return result;
}

static inline ZiVec3 zi_mat3_mul_vec3(const ZiMat3* m, ZiVec3 v) {
    return (ZiVec3){
        zi_mat3_at(m, 0, 0) * v.x + zi_mat3_at(m, 0, 1) * v.y + zi_mat3_at(m, 0, 2) * v.z,
        zi_mat3_at(m, 1, 0) * v.x + zi_mat3_at(m, 1, 1) * v.y + zi_mat3_at(m, 1, 2) * v.z,
        zi_mat3_at(m, 2, 0) * v.x + zi_mat3_at(m, 2, 1) * v.y + zi_mat3_at(m, 2, 2) * v.z
    };
}

static inline ZiMat3 zi_mat3_transpose(const ZiMat3* m) {
    ZiMat3 result;
    for (i32 row = 0; row < 3; row++) {
        for (i32 col = 0; col < 3; col++) {
            zi_mat3_set(&result, row, col, zi_mat3_at(m, col, row));
        }
    }
    return result;
}

static inline f32 zi_mat3_determinant(const ZiMat3* m) {
    return zi_mat3_at(m, 0, 0) * (zi_mat3_at(m, 1, 1) * zi_mat3_at(m, 2, 2) - zi_mat3_at(m, 2, 1) * zi_mat3_at(m, 1, 2))
         - zi_mat3_at(m, 0, 1) * (zi_mat3_at(m, 1, 0) * zi_mat3_at(m, 2, 2) - zi_mat3_at(m, 1, 2) * zi_mat3_at(m, 2, 0))
         + zi_mat3_at(m, 0, 2) * (zi_mat3_at(m, 1, 0) * zi_mat3_at(m, 2, 1) - zi_mat3_at(m, 1, 1) * zi_mat3_at(m, 2, 0));
}

static inline ZiMat3 zi_mat3_inverse(const ZiMat3* m) {
    f32 det = zi_mat3_determinant(m);
    if (zi_abs_f32(det) < ZI_EPSILON) {
        return zi_mat3_identity();
    }
    f32 inv_det = 1.0f / det;
    ZiMat3 result;
    zi_mat3_set(&result, 0, 0, (zi_mat3_at(m, 1, 1) * zi_mat3_at(m, 2, 2) - zi_mat3_at(m, 2, 1) * zi_mat3_at(m, 1, 2)) * inv_det);
    zi_mat3_set(&result, 0, 1, (zi_mat3_at(m, 0, 2) * zi_mat3_at(m, 2, 1) - zi_mat3_at(m, 0, 1) * zi_mat3_at(m, 2, 2)) * inv_det);
    zi_mat3_set(&result, 0, 2, (zi_mat3_at(m, 0, 1) * zi_mat3_at(m, 1, 2) - zi_mat3_at(m, 0, 2) * zi_mat3_at(m, 1, 1)) * inv_det);
    zi_mat3_set(&result, 1, 0, (zi_mat3_at(m, 1, 2) * zi_mat3_at(m, 2, 0) - zi_mat3_at(m, 1, 0) * zi_mat3_at(m, 2, 2)) * inv_det);
    zi_mat3_set(&result, 1, 1, (zi_mat3_at(m, 0, 0) * zi_mat3_at(m, 2, 2) - zi_mat3_at(m, 0, 2) * zi_mat3_at(m, 2, 0)) * inv_det);
    zi_mat3_set(&result, 1, 2, (zi_mat3_at(m, 1, 0) * zi_mat3_at(m, 0, 2) - zi_mat3_at(m, 0, 0) * zi_mat3_at(m, 1, 2)) * inv_det);
    zi_mat3_set(&result, 2, 0, (zi_mat3_at(m, 1, 0) * zi_mat3_at(m, 2, 1) - zi_mat3_at(m, 2, 0) * zi_mat3_at(m, 1, 1)) * inv_det);
    zi_mat3_set(&result, 2, 1, (zi_mat3_at(m, 2, 0) * zi_mat3_at(m, 0, 1) - zi_mat3_at(m, 0, 0) * zi_mat3_at(m, 2, 1)) * inv_det);
    zi_mat3_set(&result, 2, 2, (zi_mat3_at(m, 0, 0) * zi_mat3_at(m, 1, 1) - zi_mat3_at(m, 1, 0) * zi_mat3_at(m, 0, 1)) * inv_det);
    return result;
}

// ============================================================================
// Matrix 3x4 (column-major, for affine transforms)
// ============================================================================

typedef struct ZiMat34 {
    f32 m[12]; // Column-major: m[col * 3 + row], 4 columns of 3 rows
} ZiMat34;

static inline ZiMat34 zi_mat34_identity(void) {
    return (ZiMat34){{
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 0.0f
    }};
}

static inline f32 zi_mat34_at(const ZiMat34* m, i32 row, i32 col) {
    return m->m[col * 3 + row];
}

static inline void zi_mat34_set(ZiMat34* m, i32 row, i32 col, f32 val) {
    m->m[col * 3 + row] = val;
}

static inline ZiVec3 zi_mat34_transform_point(const ZiMat34* m, ZiVec3 p) {
    return (ZiVec3){
        zi_mat34_at(m, 0, 0) * p.x + zi_mat34_at(m, 0, 1) * p.y + zi_mat34_at(m, 0, 2) * p.z + zi_mat34_at(m, 0, 3),
        zi_mat34_at(m, 1, 0) * p.x + zi_mat34_at(m, 1, 1) * p.y + zi_mat34_at(m, 1, 2) * p.z + zi_mat34_at(m, 1, 3),
        zi_mat34_at(m, 2, 0) * p.x + zi_mat34_at(m, 2, 1) * p.y + zi_mat34_at(m, 2, 2) * p.z + zi_mat34_at(m, 2, 3)
    };
}

static inline ZiVec3 zi_mat34_transform_vector(const ZiMat34* m, ZiVec3 v) {
    return (ZiVec3){
        zi_mat34_at(m, 0, 0) * v.x + zi_mat34_at(m, 0, 1) * v.y + zi_mat34_at(m, 0, 2) * v.z,
        zi_mat34_at(m, 1, 0) * v.x + zi_mat34_at(m, 1, 1) * v.y + zi_mat34_at(m, 1, 2) * v.z,
        zi_mat34_at(m, 2, 0) * v.x + zi_mat34_at(m, 2, 1) * v.y + zi_mat34_at(m, 2, 2) * v.z
    };
}

// ============================================================================
// Matrix 4x4 (column-major)
// ============================================================================

typedef struct ZiMat4 {
    f32 m[16]; // Column-major: m[col * 4 + row]
} ZiMat4;

static inline ZiMat4 zi_mat4_identity(void) {
    return (ZiMat4){{
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    }};
}

static inline ZiMat4 zi_mat4_zero(void) {
    return (ZiMat4){{ 0.0f }};
}

static inline f32 zi_mat4_at(const ZiMat4* m, i32 row, i32 col) {
    return m->m[col * 4 + row];
}

static inline void zi_mat4_set(ZiMat4* m, i32 row, i32 col, f32 val) {
    m->m[col * 4 + row] = val;
}

static inline ZiMat4 zi_mat4_mul(const ZiMat4* a, const ZiMat4* b) {
    ZiMat4 result = zi_mat4_zero();
    for (i32 col = 0; col < 4; col++) {
        for (i32 row = 0; row < 4; row++) {
            f32 sum = 0.0f;
            for (i32 k = 0; k < 4; k++) {
                sum += zi_mat4_at(a, row, k) * zi_mat4_at(b, k, col);
            }
            zi_mat4_set(&result, row, col, sum);
        }
    }
    return result;
}

static inline ZiVec4 zi_mat4_mul_vec4(const ZiMat4* m, ZiVec4 v) {
    return (ZiVec4){
        zi_mat4_at(m, 0, 0) * v.x + zi_mat4_at(m, 0, 1) * v.y + zi_mat4_at(m, 0, 2) * v.z + zi_mat4_at(m, 0, 3) * v.w,
        zi_mat4_at(m, 1, 0) * v.x + zi_mat4_at(m, 1, 1) * v.y + zi_mat4_at(m, 1, 2) * v.z + zi_mat4_at(m, 1, 3) * v.w,
        zi_mat4_at(m, 2, 0) * v.x + zi_mat4_at(m, 2, 1) * v.y + zi_mat4_at(m, 2, 2) * v.z + zi_mat4_at(m, 2, 3) * v.w,
        zi_mat4_at(m, 3, 0) * v.x + zi_mat4_at(m, 3, 1) * v.y + zi_mat4_at(m, 3, 2) * v.z + zi_mat4_at(m, 3, 3) * v.w
    };
}

static inline ZiVec3 zi_mat4_transform_point(const ZiMat4* m, ZiVec3 p) {
    ZiVec4 result = zi_mat4_mul_vec4(m, zi_vec4(p.x, p.y, p.z, 1.0f));
    if (zi_abs_f32(result.w) > ZI_EPSILON) {
        return zi_vec3(result.x / result.w, result.y / result.w, result.z / result.w);
    }
    return zi_vec3(result.x, result.y, result.z);
}

static inline ZiVec3 zi_mat4_transform_vector(const ZiMat4* m, ZiVec3 v) {
    ZiVec4 result = zi_mat4_mul_vec4(m, zi_vec4(v.x, v.y, v.z, 0.0f));
    return zi_vec3(result.x, result.y, result.z);
}

static inline ZiMat4 zi_mat4_transpose(const ZiMat4* m) {
    ZiMat4 result;
    for (i32 row = 0; row < 4; row++) {
        for (i32 col = 0; col < 4; col++) {
            zi_mat4_set(&result, row, col, zi_mat4_at(m, col, row));
        }
    }
    return result;
}

static inline ZiMat4 zi_mat4_translate(ZiVec3 t) {
    ZiMat4 m = zi_mat4_identity();
    zi_mat4_set(&m, 0, 3, t.x);
    zi_mat4_set(&m, 1, 3, t.y);
    zi_mat4_set(&m, 2, 3, t.z);
    return m;
}

static inline ZiMat4 zi_mat4_scale(ZiVec3 s) {
    ZiMat4 m = zi_mat4_identity();
    zi_mat4_set(&m, 0, 0, s.x);
    zi_mat4_set(&m, 1, 1, s.y);
    zi_mat4_set(&m, 2, 2, s.z);
    return m;
}

static inline ZiMat4 zi_mat4_rotate_x(f32 radians) {
    f32 c = cosf(radians);
    f32 s = sinf(radians);
    ZiMat4 m = zi_mat4_identity();
    zi_mat4_set(&m, 1, 1, c);
    zi_mat4_set(&m, 1, 2, -s);
    zi_mat4_set(&m, 2, 1, s);
    zi_mat4_set(&m, 2, 2, c);
    return m;
}

static inline ZiMat4 zi_mat4_rotate_y(f32 radians) {
    f32 c = cosf(radians);
    f32 s = sinf(radians);
    ZiMat4 m = zi_mat4_identity();
    zi_mat4_set(&m, 0, 0, c);
    zi_mat4_set(&m, 0, 2, s);
    zi_mat4_set(&m, 2, 0, -s);
    zi_mat4_set(&m, 2, 2, c);
    return m;
}

static inline ZiMat4 zi_mat4_rotate_z(f32 radians) {
    f32 c = cosf(radians);
    f32 s = sinf(radians);
    ZiMat4 m = zi_mat4_identity();
    zi_mat4_set(&m, 0, 0, c);
    zi_mat4_set(&m, 0, 1, -s);
    zi_mat4_set(&m, 1, 0, s);
    zi_mat4_set(&m, 1, 1, c);
    return m;
}

static inline ZiMat4 zi_mat4_rotate_axis(ZiVec3 axis, f32 radians) {
    ZiVec3 n = zi_vec3_normalize(axis);
    f32 c = cosf(radians);
    f32 s = sinf(radians);
    f32 t = 1.0f - c;

    ZiMat4 m = zi_mat4_identity();
    zi_mat4_set(&m, 0, 0, t * n.x * n.x + c);
    zi_mat4_set(&m, 0, 1, t * n.x * n.y - s * n.z);
    zi_mat4_set(&m, 0, 2, t * n.x * n.z + s * n.y);
    zi_mat4_set(&m, 1, 0, t * n.x * n.y + s * n.z);
    zi_mat4_set(&m, 1, 1, t * n.y * n.y + c);
    zi_mat4_set(&m, 1, 2, t * n.y * n.z - s * n.x);
    zi_mat4_set(&m, 2, 0, t * n.x * n.z - s * n.y);
    zi_mat4_set(&m, 2, 1, t * n.y * n.z + s * n.x);
    zi_mat4_set(&m, 2, 2, t * n.z * n.z + c);
    return m;
}

static inline ZiMat4 zi_mat4_look_at(ZiVec3 eye, ZiVec3 target, ZiVec3 up) {
    ZiVec3 f = zi_vec3_normalize(zi_vec3_sub(target, eye));
    ZiVec3 r = zi_vec3_normalize(zi_vec3_cross(f, up));
    ZiVec3 u = zi_vec3_cross(r, f);

    ZiMat4 m = zi_mat4_identity();
    zi_mat4_set(&m, 0, 0, r.x);
    zi_mat4_set(&m, 0, 1, r.y);
    zi_mat4_set(&m, 0, 2, r.z);
    zi_mat4_set(&m, 1, 0, u.x);
    zi_mat4_set(&m, 1, 1, u.y);
    zi_mat4_set(&m, 1, 2, u.z);
    zi_mat4_set(&m, 2, 0, -f.x);
    zi_mat4_set(&m, 2, 1, -f.y);
    zi_mat4_set(&m, 2, 2, -f.z);
    zi_mat4_set(&m, 0, 3, -zi_vec3_dot(r, eye));
    zi_mat4_set(&m, 1, 3, -zi_vec3_dot(u, eye));
    zi_mat4_set(&m, 2, 3, zi_vec3_dot(f, eye));
    return m;
}

static inline ZiMat4 zi_mat4_perspective(f32 fov_y_radians, f32 aspect, f32 near_plane, f32 far_plane) {
    f32 tan_half_fov = tanf(fov_y_radians * 0.5f);
    ZiMat4 m = zi_mat4_zero();
    zi_mat4_set(&m, 0, 0, 1.0f / (aspect * tan_half_fov));
    zi_mat4_set(&m, 1, 1, 1.0f / tan_half_fov);
    zi_mat4_set(&m, 2, 2, -(far_plane + near_plane) / (far_plane - near_plane));
    zi_mat4_set(&m, 2, 3, -(2.0f * far_plane * near_plane) / (far_plane - near_plane));
    zi_mat4_set(&m, 3, 2, -1.0f);
    return m;
}

static inline ZiMat4 zi_mat4_ortho(f32 left, f32 right, f32 bottom, f32 top, f32 near_plane, f32 far_plane) {
    ZiMat4 m = zi_mat4_identity();
    zi_mat4_set(&m, 0, 0, 2.0f / (right - left));
    zi_mat4_set(&m, 1, 1, 2.0f / (top - bottom));
    zi_mat4_set(&m, 2, 2, -2.0f / (far_plane - near_plane));
    zi_mat4_set(&m, 0, 3, -(right + left) / (right - left));
    zi_mat4_set(&m, 1, 3, -(top + bottom) / (top - bottom));
    zi_mat4_set(&m, 2, 3, -(far_plane + near_plane) / (far_plane - near_plane));
    return m;
}

static inline ZiMat4 zi_mat4_inverse(const ZiMat4* m) {
    f32 a00 = zi_mat4_at(m, 0, 0), a01 = zi_mat4_at(m, 0, 1), a02 = zi_mat4_at(m, 0, 2), a03 = zi_mat4_at(m, 0, 3);
    f32 a10 = zi_mat4_at(m, 1, 0), a11 = zi_mat4_at(m, 1, 1), a12 = zi_mat4_at(m, 1, 2), a13 = zi_mat4_at(m, 1, 3);
    f32 a20 = zi_mat4_at(m, 2, 0), a21 = zi_mat4_at(m, 2, 1), a22 = zi_mat4_at(m, 2, 2), a23 = zi_mat4_at(m, 2, 3);
    f32 a30 = zi_mat4_at(m, 3, 0), a31 = zi_mat4_at(m, 3, 1), a32 = zi_mat4_at(m, 3, 2), a33 = zi_mat4_at(m, 3, 3);

    f32 b00 = a00 * a11 - a01 * a10;
    f32 b01 = a00 * a12 - a02 * a10;
    f32 b02 = a00 * a13 - a03 * a10;
    f32 b03 = a01 * a12 - a02 * a11;
    f32 b04 = a01 * a13 - a03 * a11;
    f32 b05 = a02 * a13 - a03 * a12;
    f32 b06 = a20 * a31 - a21 * a30;
    f32 b07 = a20 * a32 - a22 * a30;
    f32 b08 = a20 * a33 - a23 * a30;
    f32 b09 = a21 * a32 - a22 * a31;
    f32 b10 = a21 * a33 - a23 * a31;
    f32 b11 = a22 * a33 - a23 * a32;

    f32 det = b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06;
    if (zi_abs_f32(det) < ZI_EPSILON) {
        return zi_mat4_identity();
    }
    f32 inv_det = 1.0f / det;

    ZiMat4 result;
    zi_mat4_set(&result, 0, 0, ( a11 * b11 - a12 * b10 + a13 * b09) * inv_det);
    zi_mat4_set(&result, 0, 1, (-a01 * b11 + a02 * b10 - a03 * b09) * inv_det);
    zi_mat4_set(&result, 0, 2, ( a31 * b05 - a32 * b04 + a33 * b03) * inv_det);
    zi_mat4_set(&result, 0, 3, (-a21 * b05 + a22 * b04 - a23 * b03) * inv_det);
    zi_mat4_set(&result, 1, 0, (-a10 * b11 + a12 * b08 - a13 * b07) * inv_det);
    zi_mat4_set(&result, 1, 1, ( a00 * b11 - a02 * b08 + a03 * b07) * inv_det);
    zi_mat4_set(&result, 1, 2, (-a30 * b05 + a32 * b02 - a33 * b01) * inv_det);
    zi_mat4_set(&result, 1, 3, ( a20 * b05 - a22 * b02 + a23 * b01) * inv_det);
    zi_mat4_set(&result, 2, 0, ( a10 * b10 - a11 * b08 + a13 * b06) * inv_det);
    zi_mat4_set(&result, 2, 1, (-a00 * b10 + a01 * b08 - a03 * b06) * inv_det);
    zi_mat4_set(&result, 2, 2, ( a30 * b04 - a31 * b02 + a33 * b00) * inv_det);
    zi_mat4_set(&result, 2, 3, (-a20 * b04 + a21 * b02 - a23 * b00) * inv_det);
    zi_mat4_set(&result, 3, 0, (-a10 * b09 + a11 * b07 - a12 * b06) * inv_det);
    zi_mat4_set(&result, 3, 1, ( a00 * b09 - a01 * b07 + a02 * b06) * inv_det);
    zi_mat4_set(&result, 3, 2, (-a30 * b03 + a31 * b01 - a32 * b00) * inv_det);
    zi_mat4_set(&result, 3, 3, ( a20 * b03 - a21 * b01 + a22 * b00) * inv_det);
    return result;
}

// Extract upper-left 3x3 from 4x4
static inline ZiMat3 zi_mat4_to_mat3(const ZiMat4* m) {
    ZiMat3 result;
    for (i32 col = 0; col < 3; col++) {
        for (i32 row = 0; row < 3; row++) {
            zi_mat3_set(&result, row, col, zi_mat4_at(m, row, col));
        }
    }
    return result;
}

// ============================================================================
// Quaternion
// ============================================================================

typedef struct ZiQuat {
    f32 x, y, z, w;
} ZiQuat;

static inline ZiQuat zi_quat(f32 x, f32 y, f32 z, f32 w) {
    return (ZiQuat){ x, y, z, w };
}

static inline ZiQuat zi_quat_identity(void) {
    return (ZiQuat){ 0.0f, 0.0f, 0.0f, 1.0f };
}

static inline ZiQuat zi_quat_from_axis_angle(ZiVec3 axis, f32 radians) {
    ZiVec3 n = zi_vec3_normalize(axis);
    f32 half = radians * 0.5f;
    f32 s = sinf(half);
    return (ZiQuat){ n.x * s, n.y * s, n.z * s, cosf(half) };
}

static inline ZiQuat zi_quat_from_euler(f32 pitch, f32 yaw, f32 roll) {
    f32 cp = cosf(pitch * 0.5f);
    f32 sp = sinf(pitch * 0.5f);
    f32 cy = cosf(yaw * 0.5f);
    f32 sy = sinf(yaw * 0.5f);
    f32 cr = cosf(roll * 0.5f);
    f32 sr = sinf(roll * 0.5f);
    return (ZiQuat){
        sr * cp * cy - cr * sp * sy,
        cr * sp * cy + sr * cp * sy,
        cr * cp * sy - sr * sp * cy,
        cr * cp * cy + sr * sp * sy
    };
}

static inline f32 zi_quat_length_sq(ZiQuat q) {
    return q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
}

static inline f32 zi_quat_length(ZiQuat q) {
    return sqrtf(zi_quat_length_sq(q));
}

static inline ZiQuat zi_quat_normalize(ZiQuat q) {
    f32 len = zi_quat_length(q);
    if (len > ZI_EPSILON) {
        f32 inv = 1.0f / len;
        return (ZiQuat){ q.x * inv, q.y * inv, q.z * inv, q.w * inv };
    }
    return zi_quat_identity();
}

static inline ZiQuat zi_quat_conjugate(ZiQuat q) {
    return (ZiQuat){ -q.x, -q.y, -q.z, q.w };
}

static inline ZiQuat zi_quat_inverse(ZiQuat q) {
    f32 len_sq = zi_quat_length_sq(q);
    if (len_sq > ZI_EPSILON) {
        f32 inv = 1.0f / len_sq;
        return (ZiQuat){ -q.x * inv, -q.y * inv, -q.z * inv, q.w * inv };
    }
    return zi_quat_identity();
}

static inline ZiQuat zi_quat_mul(ZiQuat a, ZiQuat b) {
    return (ZiQuat){
        a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
        a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,
        a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w,
        a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z
    };
}

static inline ZiVec3 zi_quat_rotate_vec3(ZiQuat q, ZiVec3 v) {
    ZiVec3 qv = { q.x, q.y, q.z };
    ZiVec3 uv = zi_vec3_cross(qv, v);
    ZiVec3 uuv = zi_vec3_cross(qv, uv);
    uv = zi_vec3_scale(uv, 2.0f * q.w);
    uuv = zi_vec3_scale(uuv, 2.0f);
    return zi_vec3_add(zi_vec3_add(v, uv), uuv);
}

static inline f32 zi_quat_dot(ZiQuat a, ZiQuat b) {
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

static inline ZiQuat zi_quat_slerp(ZiQuat a, ZiQuat b, f32 t) {
    f32 dot = zi_quat_dot(a, b);

    // If dot is negative, negate one quaternion to take shorter path
    if (dot < 0.0f) {
        b = (ZiQuat){ -b.x, -b.y, -b.z, -b.w };
        dot = -dot;
    }

    // If very close, use linear interpolation
    if (dot > 0.9995f) {
        ZiQuat result = {
            a.x + t * (b.x - a.x),
            a.y + t * (b.y - a.y),
            a.z + t * (b.z - a.z),
            a.w + t * (b.w - a.w)
        };
        return zi_quat_normalize(result);
    }

    f32 theta_0 = acosf(dot);
    f32 theta = theta_0 * t;
    f32 sin_theta = sinf(theta);
    f32 sin_theta_0 = sinf(theta_0);

    f32 s0 = cosf(theta) - dot * sin_theta / sin_theta_0;
    f32 s1 = sin_theta / sin_theta_0;

    return (ZiQuat){
        s0 * a.x + s1 * b.x,
        s0 * a.y + s1 * b.y,
        s0 * a.z + s1 * b.z,
        s0 * a.w + s1 * b.w
    };
}

static inline ZiMat4 zi_quat_to_mat4(ZiQuat q) {
    f32 xx = q.x * q.x;
    f32 yy = q.y * q.y;
    f32 zz = q.z * q.z;
    f32 xy = q.x * q.y;
    f32 xz = q.x * q.z;
    f32 yz = q.y * q.z;
    f32 wx = q.w * q.x;
    f32 wy = q.w * q.y;
    f32 wz = q.w * q.z;

    ZiMat4 m = zi_mat4_identity();
    zi_mat4_set(&m, 0, 0, 1.0f - 2.0f * (yy + zz));
    zi_mat4_set(&m, 0, 1, 2.0f * (xy - wz));
    zi_mat4_set(&m, 0, 2, 2.0f * (xz + wy));
    zi_mat4_set(&m, 1, 0, 2.0f * (xy + wz));
    zi_mat4_set(&m, 1, 1, 1.0f - 2.0f * (xx + zz));
    zi_mat4_set(&m, 1, 2, 2.0f * (yz - wx));
    zi_mat4_set(&m, 2, 0, 2.0f * (xz - wy));
    zi_mat4_set(&m, 2, 1, 2.0f * (yz + wx));
    zi_mat4_set(&m, 2, 2, 1.0f - 2.0f * (xx + yy));
    return m;
}

static inline ZiQuat zi_quat_from_mat4(const ZiMat4* m) {
    f32 trace = zi_mat4_at(m, 0, 0) + zi_mat4_at(m, 1, 1) + zi_mat4_at(m, 2, 2);
    ZiQuat q;

    if (trace > 0.0f) {
        f32 s = 0.5f / sqrtf(trace + 1.0f);
        q.w = 0.25f / s;
        q.x = (zi_mat4_at(m, 2, 1) - zi_mat4_at(m, 1, 2)) * s;
        q.y = (zi_mat4_at(m, 0, 2) - zi_mat4_at(m, 2, 0)) * s;
        q.z = (zi_mat4_at(m, 1, 0) - zi_mat4_at(m, 0, 1)) * s;
    } else if (zi_mat4_at(m, 0, 0) > zi_mat4_at(m, 1, 1) && zi_mat4_at(m, 0, 0) > zi_mat4_at(m, 2, 2)) {
        f32 s = 2.0f * sqrtf(1.0f + zi_mat4_at(m, 0, 0) - zi_mat4_at(m, 1, 1) - zi_mat4_at(m, 2, 2));
        q.w = (zi_mat4_at(m, 2, 1) - zi_mat4_at(m, 1, 2)) / s;
        q.x = 0.25f * s;
        q.y = (zi_mat4_at(m, 0, 1) + zi_mat4_at(m, 1, 0)) / s;
        q.z = (zi_mat4_at(m, 0, 2) + zi_mat4_at(m, 2, 0)) / s;
    } else if (zi_mat4_at(m, 1, 1) > zi_mat4_at(m, 2, 2)) {
        f32 s = 2.0f * sqrtf(1.0f + zi_mat4_at(m, 1, 1) - zi_mat4_at(m, 0, 0) - zi_mat4_at(m, 2, 2));
        q.w = (zi_mat4_at(m, 0, 2) - zi_mat4_at(m, 2, 0)) / s;
        q.x = (zi_mat4_at(m, 0, 1) + zi_mat4_at(m, 1, 0)) / s;
        q.y = 0.25f * s;
        q.z = (zi_mat4_at(m, 1, 2) + zi_mat4_at(m, 2, 1)) / s;
    } else {
        f32 s = 2.0f * sqrtf(1.0f + zi_mat4_at(m, 2, 2) - zi_mat4_at(m, 0, 0) - zi_mat4_at(m, 1, 1));
        q.w = (zi_mat4_at(m, 1, 0) - zi_mat4_at(m, 0, 1)) / s;
        q.x = (zi_mat4_at(m, 0, 2) + zi_mat4_at(m, 2, 0)) / s;
        q.y = (zi_mat4_at(m, 1, 2) + zi_mat4_at(m, 2, 1)) / s;
        q.z = 0.25f * s;
    }
    return zi_quat_normalize(q);
}

// ============================================================================
// Geometric Primitives
// ============================================================================

typedef struct ZiRay {
    ZiVec3 origin;
    ZiVec3 direction;
} ZiRay;

static inline ZiRay zi_ray(ZiVec3 origin, ZiVec3 direction) {
    return (ZiRay){ origin, zi_vec3_normalize(direction) };
}

static inline ZiVec3 zi_ray_point_at(ZiRay r, f32 t) {
    return zi_vec3_add(r.origin, zi_vec3_scale(r.direction, t));
}

typedef struct ZiPlane {
    ZiVec3 normal;
    f32 distance;
} ZiPlane;

static inline ZiPlane zi_plane(ZiVec3 normal, f32 distance) {
    return (ZiPlane){ zi_vec3_normalize(normal), distance };
}

static inline ZiPlane zi_plane_from_point_normal(ZiVec3 point, ZiVec3 normal) {
    ZiVec3 n = zi_vec3_normalize(normal);
    return (ZiPlane){ n, zi_vec3_dot(n, point) };
}

static inline f32 zi_plane_distance_to_point(ZiPlane p, ZiVec3 point) {
    return zi_vec3_dot(p.normal, point) - p.distance;
}

typedef struct ZiSphere {
    ZiVec3 center;
    f32 radius;
} ZiSphere;

static inline ZiSphere zi_sphere(ZiVec3 center, f32 radius) {
    return (ZiSphere){ center, radius };
}

typedef struct ZiAABB {
    ZiVec3 min;
    ZiVec3 max;
} ZiAABB;

static inline ZiAABB zi_aabb(ZiVec3 min, ZiVec3 max) {
    return (ZiAABB){ min, max };
}

static inline ZiVec3 zi_aabb_center(ZiAABB b) {
    return zi_vec3_scale(zi_vec3_add(b.min, b.max), 0.5f);
}

static inline ZiVec3 zi_aabb_extents(ZiAABB b) {
    return zi_vec3_scale(zi_vec3_sub(b.max, b.min), 0.5f);
}

static inline ZiAABB zi_aabb_expand(ZiAABB b, ZiVec3 point) {
    return (ZiAABB){
        zi_vec3(zi_min_f32(b.min.x, point.x), zi_min_f32(b.min.y, point.y), zi_min_f32(b.min.z, point.z)),
        zi_vec3(zi_max_f32(b.max.x, point.x), zi_max_f32(b.max.y, point.y), zi_max_f32(b.max.z, point.z))
    };
}

static inline i32 zi_aabb_contains_point(ZiAABB b, ZiVec3 point) {
    return point.x >= b.min.x && point.x <= b.max.x &&
           point.y >= b.min.y && point.y <= b.max.y &&
           point.z >= b.min.z && point.z <= b.max.z;
}

typedef struct ZiTriangle {
    ZiVec3 v0, v1, v2;
} ZiTriangle;

static inline ZiTriangle zi_triangle(ZiVec3 v0, ZiVec3 v1, ZiVec3 v2) {
    return (ZiTriangle){ v0, v1, v2 };
}

static inline ZiVec3 zi_triangle_normal(ZiTriangle t) {
    ZiVec3 e1 = zi_vec3_sub(t.v1, t.v0);
    ZiVec3 e2 = zi_vec3_sub(t.v2, t.v0);
    return zi_vec3_normalize(zi_vec3_cross(e1, e2));
}

typedef struct ZiFrustum {
    ZiPlane planes[6]; // Left, Right, Bottom, Top, Near, Far
} ZiFrustum;

static inline ZiFrustum zi_frustum_from_mat4(const ZiMat4* vp) {
    ZiFrustum f;
    // Left
    f.planes[0].normal.x = zi_mat4_at(vp, 0, 3) + zi_mat4_at(vp, 0, 0);
    f.planes[0].normal.y = zi_mat4_at(vp, 1, 3) + zi_mat4_at(vp, 1, 0);
    f.planes[0].normal.z = zi_mat4_at(vp, 2, 3) + zi_mat4_at(vp, 2, 0);
    f.planes[0].distance = zi_mat4_at(vp, 3, 3) + zi_mat4_at(vp, 3, 0);
    // Right
    f.planes[1].normal.x = zi_mat4_at(vp, 0, 3) - zi_mat4_at(vp, 0, 0);
    f.planes[1].normal.y = zi_mat4_at(vp, 1, 3) - zi_mat4_at(vp, 1, 0);
    f.planes[1].normal.z = zi_mat4_at(vp, 2, 3) - zi_mat4_at(vp, 2, 0);
    f.planes[1].distance = zi_mat4_at(vp, 3, 3) - zi_mat4_at(vp, 3, 0);
    // Bottom
    f.planes[2].normal.x = zi_mat4_at(vp, 0, 3) + zi_mat4_at(vp, 0, 1);
    f.planes[2].normal.y = zi_mat4_at(vp, 1, 3) + zi_mat4_at(vp, 1, 1);
    f.planes[2].normal.z = zi_mat4_at(vp, 2, 3) + zi_mat4_at(vp, 2, 1);
    f.planes[2].distance = zi_mat4_at(vp, 3, 3) + zi_mat4_at(vp, 3, 1);
    // Top
    f.planes[3].normal.x = zi_mat4_at(vp, 0, 3) - zi_mat4_at(vp, 0, 1);
    f.planes[3].normal.y = zi_mat4_at(vp, 1, 3) - zi_mat4_at(vp, 1, 1);
    f.planes[3].normal.z = zi_mat4_at(vp, 2, 3) - zi_mat4_at(vp, 2, 1);
    f.planes[3].distance = zi_mat4_at(vp, 3, 3) - zi_mat4_at(vp, 3, 1);
    // Near
    f.planes[4].normal.x = zi_mat4_at(vp, 0, 3) + zi_mat4_at(vp, 0, 2);
    f.planes[4].normal.y = zi_mat4_at(vp, 1, 3) + zi_mat4_at(vp, 1, 2);
    f.planes[4].normal.z = zi_mat4_at(vp, 2, 3) + zi_mat4_at(vp, 2, 2);
    f.planes[4].distance = zi_mat4_at(vp, 3, 3) + zi_mat4_at(vp, 3, 2);
    // Far
    f.planes[5].normal.x = zi_mat4_at(vp, 0, 3) - zi_mat4_at(vp, 0, 2);
    f.planes[5].normal.y = zi_mat4_at(vp, 1, 3) - zi_mat4_at(vp, 1, 2);
    f.planes[5].normal.z = zi_mat4_at(vp, 2, 3) - zi_mat4_at(vp, 2, 2);
    f.planes[5].distance = zi_mat4_at(vp, 3, 3) - zi_mat4_at(vp, 3, 2);

    // Normalize all planes
    for (i32 i = 0; i < 6; i++) {
        f32 len = zi_vec3_length(f.planes[i].normal);
        if (len > ZI_EPSILON) {
            f.planes[i].normal = zi_vec3_scale(f.planes[i].normal, 1.0f / len);
            f.planes[i].distance /= len;
        }
    }
    return f;
}

// ============================================================================
// Intersection Tests
// ============================================================================

// Ray-Plane intersection
// Returns 1 if intersects, 0 otherwise. t_out is the distance along the ray.
static inline i32 zi_ray_plane_intersect(ZiRay ray, ZiPlane plane, f32* t_out) {
    f32 denom = zi_vec3_dot(plane.normal, ray.direction);
    if (zi_abs_f32(denom) < ZI_EPSILON) {
        return 0;
    }
    f32 t = (plane.distance - zi_vec3_dot(plane.normal, ray.origin)) / denom;
    if (t < 0.0f) {
        return 0;
    }
    if (t_out) *t_out = t;
    return 1;
}

// Ray-Sphere intersection
static inline i32 zi_ray_sphere_intersect(ZiRay ray, ZiSphere sphere, f32* t_out) {
    ZiVec3 oc = zi_vec3_sub(ray.origin, sphere.center);
    f32 a = zi_vec3_dot(ray.direction, ray.direction);
    f32 b = 2.0f * zi_vec3_dot(oc, ray.direction);
    f32 c = zi_vec3_dot(oc, oc) - sphere.radius * sphere.radius;
    f32 discriminant = b * b - 4.0f * a * c;

    if (discriminant < 0.0f) {
        return 0;
    }

    f32 sqrt_d = sqrtf(discriminant);
    f32 t = (-b - sqrt_d) / (2.0f * a);
    if (t < 0.0f) {
        t = (-b + sqrt_d) / (2.0f * a);
        if (t < 0.0f) {
            return 0;
        }
    }
    if (t_out) *t_out = t;
    return 1;
}

// Ray-AABB intersection (slab method)
static inline i32 zi_ray_aabb_intersect(ZiRay ray, ZiAABB aabb, f32* t_out) {
    f32 t_min = 0.0f;
    f32 t_max = F32_MAX;

    for (i32 i = 0; i < 3; i++) {
        f32 origin = (&ray.origin.x)[i];
        f32 dir = (&ray.direction.x)[i];
        f32 aabb_min = (&aabb.min.x)[i];
        f32 aabb_max = (&aabb.max.x)[i];

        if (zi_abs_f32(dir) < ZI_EPSILON) {
            if (origin < aabb_min || origin > aabb_max) {
                return 0;
            }
        } else {
            f32 inv_d = 1.0f / dir;
            f32 t1 = (aabb_min - origin) * inv_d;
            f32 t2 = (aabb_max - origin) * inv_d;
            if (t1 > t2) {
                f32 tmp = t1; t1 = t2; t2 = tmp;
            }
            t_min = zi_max_f32(t_min, t1);
            t_max = zi_min_f32(t_max, t2);
            if (t_min > t_max) {
                return 0;
            }
        }
    }
    if (t_out) *t_out = t_min;
    return 1;
}

// Ray-Triangle intersection (Moller-Trumbore algorithm)
static inline i32 zi_ray_triangle_intersect(ZiRay ray, ZiTriangle tri, f32* t_out, f32* u_out, f32* v_out) {
    ZiVec3 e1 = zi_vec3_sub(tri.v1, tri.v0);
    ZiVec3 e2 = zi_vec3_sub(tri.v2, tri.v0);
    ZiVec3 h = zi_vec3_cross(ray.direction, e2);
    f32 a = zi_vec3_dot(e1, h);

    if (zi_abs_f32(a) < ZI_EPSILON) {
        return 0;
    }

    f32 f = 1.0f / a;
    ZiVec3 s = zi_vec3_sub(ray.origin, tri.v0);
    f32 u = f * zi_vec3_dot(s, h);

    if (u < 0.0f || u > 1.0f) {
        return 0;
    }

    ZiVec3 q = zi_vec3_cross(s, e1);
    f32 v = f * zi_vec3_dot(ray.direction, q);

    if (v < 0.0f || u + v > 1.0f) {
        return 0;
    }

    f32 t = f * zi_vec3_dot(e2, q);

    if (t < ZI_EPSILON) {
        return 0;
    }

    if (t_out) *t_out = t;
    if (u_out) *u_out = u;
    if (v_out) *v_out = v;
    return 1;
}

// AABB-AABB intersection
static inline i32 zi_aabb_aabb_intersect(ZiAABB a, ZiAABB b) {
    return a.min.x <= b.max.x && a.max.x >= b.min.x &&
           a.min.y <= b.max.y && a.max.y >= b.min.y &&
           a.min.z <= b.max.z && a.max.z >= b.min.z;
}

// Sphere-Sphere intersection
static inline i32 zi_sphere_sphere_intersect(ZiSphere a, ZiSphere b) {
    ZiVec3 d = zi_vec3_sub(a.center, b.center);
    f32 dist_sq = zi_vec3_length_sq(d);
    f32 radius_sum = a.radius + b.radius;
    return dist_sq <= radius_sum * radius_sum;
}

// Frustum-Point test
static inline i32 zi_frustum_contains_point(ZiFrustum f, ZiVec3 point) {
    for (i32 i = 0; i < 6; i++) {
        if (zi_plane_distance_to_point(f.planes[i], point) < 0.0f) {
            return 0;
        }
    }
    return 1;
}

// Frustum-Sphere test
static inline i32 zi_frustum_contains_sphere(ZiFrustum f, ZiSphere s) {
    for (i32 i = 0; i < 6; i++) {
        if (zi_plane_distance_to_point(f.planes[i], s.center) < -s.radius) {
            return 0;
        }
    }
    return 1;
}

// Frustum-AABB test
static inline i32 zi_frustum_contains_aabb(ZiFrustum f, ZiAABB b) {
    for (i32 i = 0; i < 6; i++) {
        ZiVec3 p;
        p.x = f.planes[i].normal.x > 0.0f ? b.max.x : b.min.x;
        p.y = f.planes[i].normal.y > 0.0f ? b.max.y : b.min.y;
        p.z = f.planes[i].normal.z > 0.0f ? b.max.z : b.min.z;
        if (zi_plane_distance_to_point(f.planes[i], p) < 0.0f) {
            return 0;
        }
    }
    return 1;
}

// ============================================================================
// Transform Decomposition
// ============================================================================

static inline void zi_mat4_decompose(const ZiMat4* m, ZiVec3* translation, ZiQuat* rotation, ZiVec3* scale) {
    if (translation) {
        translation->x = zi_mat4_at(m, 0, 3);
        translation->y = zi_mat4_at(m, 1, 3);
        translation->z = zi_mat4_at(m, 2, 3);
    }

    if (scale) {
        scale->x = zi_vec3_length(zi_vec3(zi_mat4_at(m, 0, 0), zi_mat4_at(m, 1, 0), zi_mat4_at(m, 2, 0)));
        scale->y = zi_vec3_length(zi_vec3(zi_mat4_at(m, 0, 1), zi_mat4_at(m, 1, 1), zi_mat4_at(m, 2, 1)));
        scale->z = zi_vec3_length(zi_vec3(zi_mat4_at(m, 0, 2), zi_mat4_at(m, 1, 2), zi_mat4_at(m, 2, 2)));
    }

    if (rotation) {
        ZiVec3 s = scale ? *scale : zi_vec3(
            zi_vec3_length(zi_vec3(zi_mat4_at(m, 0, 0), zi_mat4_at(m, 1, 0), zi_mat4_at(m, 2, 0))),
            zi_vec3_length(zi_vec3(zi_mat4_at(m, 0, 1), zi_mat4_at(m, 1, 1), zi_mat4_at(m, 2, 1))),
            zi_vec3_length(zi_vec3(zi_mat4_at(m, 0, 2), zi_mat4_at(m, 1, 2), zi_mat4_at(m, 2, 2)))
        );

        ZiMat4 rot = *m;
        if (s.x > ZI_EPSILON) {
            zi_mat4_set(&rot, 0, 0, zi_mat4_at(m, 0, 0) / s.x);
            zi_mat4_set(&rot, 1, 0, zi_mat4_at(m, 1, 0) / s.x);
            zi_mat4_set(&rot, 2, 0, zi_mat4_at(m, 2, 0) / s.x);
        }
        if (s.y > ZI_EPSILON) {
            zi_mat4_set(&rot, 0, 1, zi_mat4_at(m, 0, 1) / s.y);
            zi_mat4_set(&rot, 1, 1, zi_mat4_at(m, 1, 1) / s.y);
            zi_mat4_set(&rot, 2, 1, zi_mat4_at(m, 2, 1) / s.y);
        }
        if (s.z > ZI_EPSILON) {
            zi_mat4_set(&rot, 0, 2, zi_mat4_at(m, 0, 2) / s.z);
            zi_mat4_set(&rot, 1, 2, zi_mat4_at(m, 1, 2) / s.z);
            zi_mat4_set(&rot, 2, 2, zi_mat4_at(m, 2, 2) / s.z);
        }
        zi_mat4_set(&rot, 0, 3, 0.0f);
        zi_mat4_set(&rot, 1, 3, 0.0f);
        zi_mat4_set(&rot, 2, 3, 0.0f);
        zi_mat4_set(&rot, 3, 0, 0.0f);
        zi_mat4_set(&rot, 3, 1, 0.0f);
        zi_mat4_set(&rot, 3, 2, 0.0f);
        zi_mat4_set(&rot, 3, 3, 1.0f);

        *rotation = zi_quat_from_mat4(&rot);
    }
}

static inline ZiMat4 zi_mat4_compose(ZiVec3 translation, ZiQuat rotation, ZiVec3 scale) {
    ZiMat4 t = zi_mat4_translate(translation);
    ZiMat4 r = zi_quat_to_mat4(rotation);
    ZiMat4 s = zi_mat4_scale(scale);
    ZiMat4 rs = zi_mat4_mul(&r, &s);
    return zi_mat4_mul(&t, &rs);
}
