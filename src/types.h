#ifndef TYPES_H
#define TYPES_H

#include <SDL3/SDL.h>
#include <stdbool.h>

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
  bool is_running;

  // System
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Texture *texture;
  uint32_t *pixel_buffer;
  int height, width;
  int pitch;

  // Timing
  float delta_time;
  uint64_t last_frame_time;

  // Scene Data
  vec3_t cube_pos;
  vec3_t cube_rot;
  vec3_t cube_scale;

  float rotation_speed;
  float total_rotation;

  mat4_t projection_matrix;
} game_t;

#endif
