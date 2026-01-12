#include "display.h"
#include "colors.h"
#include "math_utils.h"
#include "types.h"
#include <SDL3/SDL_stdinc.h>
#include <float.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

static uint32_t *framebuffer = NULL;
static float *depthbuffer = NULL;

static int screen_width = 0;
static int screen_height = 0;
static int screen_pitch = 0;

void set_render_target(game_t *g) {
  framebuffer = g->pixel_buffer;
  depthbuffer = g->depth_buffer;

  screen_width = g->width;
  screen_height = g->height;
  screen_pitch = g->pitch;
}

void clear_screen(uint32_t color) {
  SDL_memset4(framebuffer, color, screen_width * screen_height);
  memset(depthbuffer, 0, screen_width * screen_height * sizeof(float));
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

void draw_line(vertex_t v0, vertex_t v1, uint32_t color) {
  int x0 = (int)v0.pos.x;
  int y0 = (int)v0.pos.y;
  int x1 = (int)v1.pos.x;
  int y1 = (int)v1.pos.y;

  float w0 = v1.pos.z;
  float w1 = v1.pos.z;

  int dx = abs(x1 - x0);
  int dy = abs(y1 - y0);

  int side_length = (abs(dx) >= abs(dy)) ? abs(dx) : abs(dy);

  float w_step = (side_length == 0) ? 0.0f : (w1 - w0) / side_length;
  float current_w = w0;

  int sx = (x0 < x1) ? 1 : -1;
  int sy = (y0 < y1) ? 1 : -1;

  int err = dx - dy;

  while (1) {
    if (x0 >= 0 && x0 < screen_width && y0 >= 0 && y0 < screen_height) {
      if (current_w > depthbuffer[y0 * screen_width + x0]) {
        depthbuffer[y0 * screen_width + x0] = current_w;
        draw_pixel(x0, y0, color);
      }
    }

    current_w += w_step;

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

void draw_scanline(int x0, int x1, int y, float w0, float w1, uint32_t color) {
  if (y < 0 || y >= screen_height)
    return;

  if (x0 > x1) {
    int temp = x0;
    x0 = x1;
    x1 = temp;

    float tempW = w0;
    w0 = w1;
    w1 = tempW;
  }

  if (x0 >= screen_width)
    return;
  if (x1 < 0)
    return;

  if (x0 < 0)
    x0 = 0;
  if (x1 >= screen_width)
    x1 = screen_width - 1;

  float w_step = (w1 - w0) / (float)(x1 - x0);
  float current_w = w0;

  uint32_t *pixel_row = framebuffer + (y * (screen_pitch / 4));
  float *depth_row =
      depthbuffer + (y * screen_width); // Point to correct row in Z-buffer

  for (int x = x0; x <= x1; x++) {
    if (current_w > depth_row[x]) {
      depth_row[x] = current_w;
      pixel_row[x] = color;
    }

    current_w += w_step;
  }
}

void draw_cube(vec3_t t_m, vec3_t r_m, vec3_t s_m, game_t *g) {
  vec3_t cube_vertices[] = {{-1, -1, -1}, {1, -1, -1}, {1, 1, -1}, {-1, 1, -1},
                            {-1, -1, 1},  {1, -1, 1},  {1, 1, 1},  {-1, 1, 1}};

  int cube_indices[] = {// Front Face (Z+)
                        4, 5, 6, 4, 6, 7,

                        // Right Face (X+)
                        5, 1, 2, 5, 2, 6,

                        // Back Face (Z-)
                        1, 0, 3, 1, 3, 2,

                        // Left Face (X-)
                        0, 4, 7, 0, 7, 3,

                        // Top Face (Y+)
                        7, 6, 2, 7, 2, 3,

                        // Bottom Face (Y-)
                        0, 1, 5, 0, 5, 4};

  mat4_t scale_m = mat4_make_scale(s_m.x, s_m.y, s_m.z);

  mat4_t rotation_z = mat4_make_rotation_z(deg_to_rad(r_m.z));
  mat4_t rotation_y = mat4_make_rotation_y(deg_to_rad(r_m.y));
  mat4_t rotation_x = mat4_make_rotation_x(deg_to_rad(r_m.x));

  mat4_t rotation_m =
      mat4_mul_mat4(rotation_x, mat4_mul_mat4(rotation_y, rotation_z));

  mat4_t translation_m = mat4_make_translation(t_m.x, t_m.y, t_m.z);
  mat4_t model_m =
      mat4_mul_mat4(translation_m, mat4_mul_mat4(rotation_m, scale_m));

  mat4_t mvp_m = mat4_mul_mat4(g->projection_matrix,
                               mat4_mul_mat4(g->view_matrix, model_m));

  vec3_t projected_points[8];

  for (int i = 0; i < 8; i++) {
    vec4_t clip_pos = mat4_mul_vec4(mvp_m, vec3_to_vec4(cube_vertices[i]));

    if (clip_pos.w < 0.1f) {
      projected_points[i] = (vec3_t){-10000, -10000, 0};
      continue;
    }

    vec3_t projected = {clip_pos.x / clip_pos.w, clip_pos.y / clip_pos.w,
                        clip_pos.z / clip_pos.w};

    projected_points[i].z = 1.0f / clip_pos.w;

    projected_points[i].x = (projected.x + 1) * 0.5 * screen_width;
    projected_points[i].y = (1 - projected.y) * 0.5 * screen_height;
  }

  // 0, 3, 6, 9, ...
  for (int i = 0; i < 36; i += 3) {
    int index0 = cube_indices[i];
    int index1 = cube_indices[i + 1];
    int index2 = cube_indices[i + 2];

    vec4_t v0_world4 =
        mat4_mul_vec4(model_m, vec3_to_vec4(cube_vertices[index0]));
    vec4_t v1_world4 =
        mat4_mul_vec4(model_m, vec3_to_vec4(cube_vertices[index1]));
    vec4_t v2_world4 =
        mat4_mul_vec4(model_m, vec3_to_vec4(cube_vertices[index2]));

    vec3_t v0_world = {v0_world4.x, v0_world4.y, v0_world4.z};
    vec3_t v1_world = {v1_world4.x, v1_world4.y, v1_world4.z};
    vec3_t v2_world = {v2_world4.x, v2_world4.y, v2_world4.z};

    vec3_t triangle_normal =
        calculate_triangle_normal(v0_world, v1_world, v2_world);

    vec3_t center = calculate_triangle_center(v0_world, v1_world, v2_world);
    vec3_t view_ray = vec3_sub(center, g->camera_pos);

    float dot = vec3_dot(triangle_normal, view_ray);

    if (dot > 0) {
      continue;
    }

    triangle_t triangle;
    triangle.points[0].pos = projected_points[index0];
    triangle.points[1].pos = projected_points[index1];
    triangle.points[2].pos = projected_points[index2];

    draw_filled_triangle(&triangle, COLOR_MAGENTA);

    draw_line(triangle.points[0], triangle.points[1], COLOR_BLACK);
    draw_line(triangle.points[1], triangle.points[2], COLOR_BLACK);
    draw_line(triangle.points[2], triangle.points[0], COLOR_BLACK);
  }
}

void fillBottomFlatTriangle(vertex_t v1, vertex_t v2, vertex_t v3,
                            uint32_t color) {
  float invslope1 = (v2.pos.x - v1.pos.x) / (v2.pos.y - v1.pos.y);
  float invslope2 = (v3.pos.x - v1.pos.x) / (v3.pos.y - v1.pos.y);

  float w_slope1 = (v2.pos.z - v1.pos.z) / (v2.pos.y - v1.pos.y);
  float w_slope2 = (v3.pos.z - v1.pos.z) / (v3.pos.y - v1.pos.y);

  int scanlineY = (int)ceilf(v1.pos.y);
  int scanlineEnd = (int)ceilf(v2.pos.y) - 1;

  float y_prestep = (float)scanlineY - v1.pos.y;

  float curx1 = v1.pos.x + (invslope1 * y_prestep);
  float curx2 = v1.pos.x + (invslope2 * y_prestep);

  float curw1 = v1.pos.z + (w_slope1 * y_prestep);
  float curw2 = v1.pos.z + (w_slope2 * y_prestep);

  for (int y = scanlineY; y <= scanlineEnd; y++) {
    draw_scanline((int)curx1, (int)curx2, y, curw1, curw2, color);

    curx1 += invslope1;
    curx2 += invslope2;

    curw1 += w_slope1;
    curw2 += w_slope2;
  }
}

void fillTopFlatTriangle(vertex_t v1, vertex_t v2, vertex_t v3,
                         uint32_t color) {
  float invslope1 = (v3.pos.x - v1.pos.x) / (v3.pos.y - v1.pos.y);
  float invslope2 = (v3.pos.x - v2.pos.x) / (v3.pos.y - v2.pos.y);

  float w_slope1 = (v3.pos.z - v1.pos.z) / (v3.pos.y - v1.pos.y);
  float w_slope2 = (v3.pos.z - v2.pos.z) / (v3.pos.y - v2.pos.y);

  int scanlineY = (int)ceilf(v1.pos.y);
  int scanlineEnd = (int)ceilf(v3.pos.y) - 1;

  float y_prestep = (float)scanlineY - v1.pos.y;

  float curx1 = v1.pos.x + (invslope1 * y_prestep);
  float curx2 = v2.pos.x + (invslope2 * y_prestep);

  float curw1 = v1.pos.z + (w_slope1 * y_prestep);
  float curw2 = v2.pos.z + (w_slope2 * y_prestep);

  for (int y = scanlineY; y <= scanlineEnd; y++) {
    draw_scanline((int)curx1, (int)curx2, y, curw1, curw2, color);

    curx1 += invslope1;
    curx2 += invslope2;

    curw1 += w_slope1;
    curw2 += w_slope2;
  }
}

void draw_filled_triangle(const triangle_t *triangle, uint32_t color) {
  vertex_t v0 = triangle->points[0];
  vertex_t v1 = triangle->points[1];
  vertex_t v2 = triangle->points[2];

  if (v0.pos.y > v1.pos.y)
    swap(&v0, &v1);
  if (v0.pos.y > v2.pos.y)
    swap(&v0, &v2);
  if (v1.pos.y > v2.pos.y)
    swap(&v1, &v2);

  if (v1.pos.y == v2.pos.y) {
    fillBottomFlatTriangle(v0, v1, v2, color);
  } else if (v0.pos.y == v1.pos.y) {
    fillTopFlatTriangle(v0, v1, v2, color);
  } else {
    float t = (v1.pos.y - v0.pos.y) / (v2.pos.y - v0.pos.y);

    vertex_t v4;

    v4.pos.y = v1.pos.y;
    v4.pos.x = v0.pos.x + (v2.pos.x - v0.pos.x) * t;
    v4.pos.z = v0.pos.z + (v2.pos.z - v0.pos.z) * t;

    fillBottomFlatTriangle(v0, v1, v4, color);
    fillTopFlatTriangle(v1, v4, v2, color);
  }
}
