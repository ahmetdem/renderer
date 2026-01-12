#ifndef TYPES_H
#define TYPES_H

#include <SDL3/SDL.h>
#include <stdbool.h>
#include <stdint.h>

#define M_PI 3.14
#define RADIANS(deg) ((deg) * (M_PI / 180.0f))

/* Vector 3 */
typedef struct {
  float x, y, z;
} vec3_t;

typedef struct {
  float x, y, z, w;
} vec4_t;

/* 4x4 Matrix */
typedef struct {
  float m[16];
} mat4_t;

typedef struct {
  vec3_t pos;
} vertex_t;

typedef struct {
  vertex_t points[3];
} triangle_t;

/*Game Struct*/
typedef struct {
  bool is_running;

  // System
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Texture *texture;
  uint32_t *pixel_buffer;
  float *depth_buffer;

  int height, width;
  int pitch;

  // Timing
  float delta_time;
  uint64_t last_frame_time;
  int target_frame_rate;
  int frame_time;

  // Scene Data
  vec3_t cube_pos;
  vec3_t cube_rot;
  vec3_t cube_scale;

  triangle_t triangle;

  float rotation_speed;
  float total_rotation;

  mat4_t projection_matrix;

  // Camera
  vec3_t camera_pos;
  vec3_t camera_front;
  vec3_t camera_up;
  float camera_yaw;
  float camera_pitch;
  float sensitivity;

  mat4_t view_matrix;
} game_t;

#endif
