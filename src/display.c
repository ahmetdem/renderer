#include "display.h"
#include "colors.h"
#include "math_utils.h"
#include <SDL3/SDL_stdinc.h>
#include <time.h>

static uint32_t *framebuffer = NULL;
static int screen_width = 0;
static int screen_height = 0;
static int screen_pitch = 0;

void set_render_target(uint32_t *pixels, int width, int height, int pitch) {
  framebuffer = pixels;
  screen_width = width;
  screen_height = height;
  screen_pitch = pitch;
}

void clear_screen(uint32_t color) {
  SDL_memset4(framebuffer, color, screen_width * screen_height);
}

void draw_pixel(int x, int y, uint32_t color) {
  if (x < 0 || x >= screen_width || y < 0 || y >= screen_height)
    return;
  framebuffer[y * (screen_pitch / 4) + x] = color;
}

void draw_rect(int x, int y, int w, int h, uint32_t color) {
  for (int y2 = 0; y2 < h; y2++) {
    for (int x2 = 0; x2 < w; x2++) {
      float t = (float)x2 / w;
      draw_pixel(x + x2, y + y2, calculate_lerp(COLOR_RED, color, t));
    }
  }
}

void draw_line(int x0, int y0, int x1, int y1, uint32_t color) {
  int dx = (x1 - x0);
  int dy = (y1 - y0);

  int step_x = (dx >= 0) ? 1 : -1;
  int step_y = (dy >= 0) ? 1 : -1;

  dx = (dx >= 0) ? dx : -dx;
  dy = (dy >= 0) ? dy : -dy;

  int sx = step_x;
  int sy = step_y;

  int err = dx - dy;

  while (1) {
    draw_pixel(x0, y0, color);

    if (x0 == x1 && y0 == y1)
      break;

    int e2 = 2 * err;

    if (e2 > -dy) {
      err -= dy;
      x0 += sx;
    }

    if (e2 < dx) {
      err += dx;
      y0 += sy;
    }
  }
}

void draw_cube(vec3_t t_m, vec3_t r_m, vec3_t s_m, game_t* g) {
  vec3_t cube_vertices[] = {{-1, -1, -1}, {1, -1, -1}, {1, 1, -1}, {-1, 1, -1},
                            {-1, -1, 1},  {1, -1, 1},  {1, 1, 1},  {-1, 1, 1}};

  mat4_t scale_m = mat4_make_scale(s_m.x, s_m.y, s_m.z);

  mat4_t rotation_z = mat4_make_rotation_z(deg_to_rad(r_m.x));
  mat4_t rotation_y = mat4_make_rotation_y(deg_to_rad(r_m.x));
  mat4_t rotation_x = mat4_make_rotation_x(deg_to_rad(r_m.x));

  mat4_t rotation_m =
      mat4_mul_mat4(rotation_x, mat4_mul_mat4(rotation_y, rotation_z));

  mat4_t translation_m = mat4_make_translation(t_m.x, t_m.y, t_m.z);
  mat4_t world_m =
      mat4_mul_mat4(translation_m, mat4_mul_mat4(scale_m, rotation_m));

  vec3_t projected_points[8];

  for (int i = 0; i < 8; i++) {
    vec4_t world_pos = mat4_mul_vec4(world_m, vec3_to_vec4(cube_vertices[i]));

    if (world_pos.z > -0.1f) {
      projected_points[i] = (vec3_t){-1000, -1000, 0};
      continue;
    }

    vec3_t projected = vec3_project(
        (vec3_t){world_pos.x, world_pos.y, world_pos.z}, g->projection_matrix);

    projected_points[i].x = (projected.x + 1) * 0.5 * screen_width;
    projected_points[i].y = (1 - projected.y) * 0.5 * screen_height;
  }

  // Front Face
  draw_line(projected_points[0].x, projected_points[0].y, projected_points[1].x,
            projected_points[1].y, COLOR_WHITE);
  draw_line(projected_points[1].x, projected_points[1].y, projected_points[2].x,
            projected_points[2].y, COLOR_WHITE);
  draw_line(projected_points[2].x, projected_points[2].y, projected_points[3].x,
            projected_points[3].y, COLOR_WHITE);
  draw_line(projected_points[3].x, projected_points[3].y, projected_points[0].x,
            projected_points[0].y, COLOR_WHITE);

  // Back Face
  draw_line(projected_points[4].x, projected_points[4].y, projected_points[5].x,
            projected_points[5].y, COLOR_WHITE);
  draw_line(projected_points[5].x, projected_points[5].y, projected_points[6].x,
            projected_points[6].y, COLOR_WHITE);
  draw_line(projected_points[6].x, projected_points[6].y, projected_points[7].x,
            projected_points[7].y, COLOR_WHITE);
  draw_line(projected_points[7].x, projected_points[7].y, projected_points[4].x,
            projected_points[4].y, COLOR_WHITE);

  // Connecting the two faces
  draw_line(projected_points[0].x, projected_points[0].y, projected_points[4].x,
            projected_points[4].y, COLOR_WHITE);
  draw_line(projected_points[1].x, projected_points[1].y, projected_points[5].x,
            projected_points[5].y, COLOR_WHITE);
  draw_line(projected_points[2].x, projected_points[2].y, projected_points[6].x,
            projected_points[6].y, COLOR_WHITE);
  draw_line(projected_points[3].x, projected_points[3].y, projected_points[7].x,
            projected_points[7].y, COLOR_WHITE);
}
