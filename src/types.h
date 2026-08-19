#ifndef TYPES_H
#define TYPES_H

#include <SDL3/SDL.h>
#include <stdbool.h>
#include <stdint.h>

#define M_PI 3.14159265358979f
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
  vec3_t *vertices;
  int vertex_count;

  int *indices;
  int index_count;
} mesh_t;

typedef struct {
  vertex_t points[3];
} triangle_t;

typedef struct {
  mesh_t *mesh;
  vec3_t position;
  vec3_t rotation;
  vec3_t scale;
  uint32_t color;
  uint32_t tags;
} entity_t;

typedef enum {
  TAG_NONE = 0,
  TAG_PLAYER = 1 << 0,
  TAG_COLLIDABLE = 1 << 1,
  TAG_LIGHT = 1 << 2,
  TAG_GRAVITY = 1 << 3,
  TAG_ENEMY = 1 << 4,
} entity_tag_t;

typedef enum { BLOCK_AIR = 0, BLOCK_GRASS, BLOCK_STONE, BLOCK_DIRT } block_t;

typedef struct {
  block_t blocks[16][16][16];
  vec3_t world_pos;
  mesh_t *mesh;
  bool dirty;
} chunk_t;

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

  // Light
  vec3_t light_dir;
  float ambient_light;

  // Scene Data
  entity_t *entities;
  int entity_count;
  int entity_capacity;

  chunk_t *test_chunk;

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
  mat4_t view_projection;

  // Debug
  bool show_debug;
  int triangle_count;
} game_t;

#endif
