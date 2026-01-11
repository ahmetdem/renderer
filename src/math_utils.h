#ifndef MATH_UTILS_H
#define MATH_UTILS_H

#include "types.h"
#include <math.h>
#include <stdint.h>

static inline vec3_t vec3_add_s(const vec3_t a, float k) {
  return (vec3_t){a.x + k, a.y + k, a.z + k};
}

static inline vec3_t vec3_add_v(const vec3_t a, const vec3_t b) {
  return (vec3_t){a.x + b.x, a.y + b.y, a.z + b.z};
}

static inline vec3_t vec3_sub_s(const vec3_t a, float k) {
  return (vec3_t){a.x - k, a.y - k, a.z - k};
}

static inline vec3_t vec3_sub_v(const vec3_t a, const vec3_t b) {
  return (vec3_t){a.x - b.x, a.y - b.y, a.z - b.z};
}

static inline vec3_t vec3_mul_s(const vec3_t a, float k) {
  return (vec3_t){a.x * k, a.y * k, a.z * k};
}

// Macros
#define vec3_add(a, b)                                                         \
  _Generic((b), float: vec3_add_s, double: vec3_add_s, vec3_t: vec3_add_v)(a, b)

#define vec3_sub(a, b)                                                         \
  _Generic((b), float: vec3_sub_s, double: vec3_sub_s, vec3_t: vec3_sub_v)(a, b)

#define vec3_mul(a, b)                                                         \
  _Generic((b), float: vec3_mul_s, double: vec3_mul_s, int: vec3_mul_s)(a, b)

static inline float vec3_dot(const vec3_t a, const vec3_t b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

static inline vec3_t vec3_cross(const vec3_t a, const vec3_t b) {
  return (vec3_t){a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z,
                  a.x * b.y - a.y * b.x};
}

static inline float vec3_len(const vec3_t v) {
  return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

static inline vec3_t vec3_normalize(vec3_t v) {
  float len = vec3_len(v);
  if (len > 0.000001f) {
    return (vec3_t){v.x / len, v.y / len, v.z / len};
  }

  return (vec3_t){0, 0, 0};
}

static inline vec4_t vec3_to_vec4(vec3_t vec3) {
  return (vec4_t){vec3.x, vec3.y, vec3.z, 1.0f};
}

static inline float deg_to_rad(float degrees) {
  return degrees * (M_PI / 180.0f);
}

static inline float rad_to_deg(float radians) {
  return radians * (180.0f / M_PI);
}

static inline mat4_t mat4_identity(void) {
  mat4_t res = {0};
  res.m[0] = res.m[5] = res.m[10] = res.m[15] = 1.0f;
  return res;
}

static inline mat4_t mat4_make_scale(float sx, float sy, float sz) {
  mat4_t matrix = mat4_identity();
  matrix.m[0] = sx;
  matrix.m[5] = sy;
  matrix.m[10] = sz;
  return matrix;
}

static inline mat4_t mat4_make_translation(float tx, float ty, float tz) {
  mat4_t matrix = mat4_identity();
  matrix.m[3] = tx;
  matrix.m[7] = ty;
  matrix.m[11] = tz;

  return matrix;
}

static inline mat4_t mat4_make_rotation_x(float angle) {
  mat4_t matrix = mat4_identity();

  matrix.m[5] = cos(angle);
  matrix.m[6] = -sin(angle);
  matrix.m[9] = sin(angle);
  matrix.m[10] = cos(angle);

  return matrix;
}

static inline mat4_t mat4_make_rotation_y(float angle) {
  mat4_t matrix = mat4_identity();

  matrix.m[0] = cos(angle);
  matrix.m[2] = sin(angle);
  matrix.m[8] = -sin(angle);
  matrix.m[10] = cos(angle);

  return matrix;
}

static inline mat4_t mat4_make_rotation_z(float angle) {
  mat4_t matrix = mat4_identity();

  matrix.m[0] = cos(angle);
  matrix.m[1] = -sin(angle);
  matrix.m[4] = sin(angle);
  matrix.m[5] = cos(angle);

  return matrix;
}

static inline mat4_t mat4_mul_mat4(mat4_t a, mat4_t b) {
  mat4_t matrix;

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      matrix.m[i * 4 + j] =
          a.m[i * 4 + 0] * b.m[0 * 4 + j] + a.m[i * 4 + 1] * b.m[1 * 4 + j] +
          a.m[i * 4 + 2] * b.m[2 * 4 + j] + a.m[i * 4 + 3] * b.m[3 * 4 + j];
    }
  }

  return matrix;
}

static inline vec4_t mat4_mul_vec4(mat4_t m, vec4_t v) {
  return (vec4_t){m.m[0] * v.x + m.m[1] * v.y + m.m[2] * v.z + m.m[3] * v.w,
                  m.m[4] * v.x + m.m[5] * v.y + m.m[6] * v.z + m.m[7] * v.w,
                  m.m[8] * v.x + m.m[9] * v.y + m.m[10] * v.z + m.m[11] * v.w,
                  m.m[12] * v.x + m.m[13] * v.y + m.m[14] * v.z +
                      m.m[15] * v.w};
}

static inline vec3_t vec3_project(vec3_t v, mat4_t projection_matrix) {
  vec4_t vec4 = mat4_mul_vec4(projection_matrix, (vec4_t){v.x, v.y, v.z, 1.0f});
  return (vec3_t){vec4.x / vec4.w, vec4.y / vec4.w, vec4.z / vec4.w};
}

static inline mat4_t mat4_make_perspective(float fov, float aspect, float znear,
                                           float zfar) {
  const float DEG2RAD = acos(-1.0f) / 180;

  float tangent = tan(fov / 2 * DEG2RAD);
  float right = znear * tangent;
  float top = right / aspect;

  mat4_t matrix = {0};
  matrix.m[0] = znear / right;
  matrix.m[5] = znear / top;
  matrix.m[10] = -(zfar + znear) / (zfar - znear);
  matrix.m[11] = -1;
  matrix.m[14] = -(2 * zfar * znear) / (zfar - znear);
  matrix.m[15] = 0;

  return matrix;
}

static inline mat4_t mat4_look_at(vec3_t cam_pos, vec3_t cam_target,
                                  vec3_t cam_up) {
  vec3_t cam_dir = vec3_sub(cam_pos, cam_target);

  vec3_t cam_right = vec3_normalize(vec3_cross(cam_up, cam_dir));
  vec3_t actual_up = vec3_cross(cam_dir, cam_right);

  mat4_t left_side = mat4_identity();
  left_side.m[0] = cam_right.x;
  left_side.m[1] = cam_right.y;
  left_side.m[2] = cam_right.z;

  left_side.m[4] = actual_up.x;
  left_side.m[5] = actual_up.y;
  left_side.m[6] = actual_up.z;

  left_side.m[8] = cam_dir.x;
  left_side.m[9] = cam_dir.y;
  left_side.m[10] = cam_dir.z;

  mat4_t right_side = mat4_identity();
  right_side.m[3] = -cam_pos.x;
  right_side.m[7] = -cam_pos.y;
  right_side.m[11] = -cam_pos.z;

  return mat4_mul_mat4(left_side, right_side);
}

static inline void swap(vertex_t *a, vertex_t *b) {
  vertex_t temp = *a;
  *a = *b;
  *b = temp;
}

static inline vec3_t calculate_triangle_normal(vec3_t v0, vec3_t v1,
                                               vec3_t v2) {
  vec3_t A = vec3_sub_v(v1, v0);
  vec3_t B = vec3_sub_v(v2, v0);

  return vec3_normalize(vec3_cross(A, B));
}

static inline vec3_t calculate_triangle_center(vec3_t v0, vec3_t v1,
                                               vec3_t v2) {
  vec3_t sum = vec3_add_v(v0, vec3_add_v(v1, v2));
  return vec3_mul_s(sum, 1.0f / 3.0f);
}

#endif
