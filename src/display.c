#include "display.h"
#include "colors.h"
#include "font.h"
#include "math_utils.h"
#include "types.h"
#include <SDL3/SDL_stdinc.h>
#include <float.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

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

static inline vec3_t clip_to_screen(vec4_t clip_pos, int screen_w,
                                    int screen_h) {
  return (vec3_t){(clip_pos.x / clip_pos.w + 1.0f) * 0.5f * screen_w,
                  (1.0f - clip_pos.y / clip_pos.w) * 0.5f * screen_h,
                  1.0f / clip_pos.w};
}

bool project_point(vec3_t world_pos, game_t *g, vec3_t *out_screen_point) {
  vec4_t clip_pos = mat4_mul_vec4(g->view_projection, vec3_to_vec4(world_pos));

  if (clip_pos.w < 0.1f) {
    if (out_screen_point) {
      *out_screen_point = (vec3_t){-10000.0f, -10000.0f, 0.0f};
    }
    return false;
  }

  *out_screen_point = clip_to_screen(clip_pos, g->width, g->height);
  return true;
}

void draw_line_3d(vec3_t p0, vec3_t p1, uint32_t color, game_t *g) {
  vertex_t v0, v1;
  if (project_point(p0, g, &v0.pos) && project_point(p1, g, &v1.pos)) {
    draw_line(v0, v1, color);
  }
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

  float w0 = v0.pos.z;
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
      if (current_w > depthbuffer[y0 * screen_width + x0] - 0.0001f) {
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

  int dx = x1 - x0;
  float w_step = (dx == 0) ? 0.0f : (w1 - w0) / (float)dx;
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

void draw_entity(entity_t *entity, game_t *g) {
  draw_mesh(entity->mesh, entity->position, entity->rotation, entity->scale,
            entity->color, g);
}

void draw_mesh(mesh_t *mesh, vec3_t position, vec3_t rotation, vec3_t scale,
               uint32_t color, game_t *g) {
  mat4_t scale_m = mat4_make_scale(scale.x, scale.y, scale.z);

  mat4_t rotation_z = mat4_make_rotation_z(deg_to_rad(rotation.z));
  mat4_t rotation_y = mat4_make_rotation_y(deg_to_rad(rotation.y));
  mat4_t rotation_x = mat4_make_rotation_x(deg_to_rad(rotation.x));

  mat4_t rotation_m =
      mat4_mul_mat4(rotation_x, mat4_mul_mat4(rotation_y, rotation_z));

  mat4_t translation_m =
      mat4_make_translation(position.x, position.y, position.z);

  mat4_t model_m =
      mat4_mul_mat4(translation_m, mat4_mul_mat4(rotation_m, scale_m));

  // 0, 3, 6, 9, ...
  for (int i = 0; i < mesh->index_count; i += 3) {
    int index0 = mesh->indices[i];
    int index1 = mesh->indices[i + 1];
    int index2 = mesh->indices[i + 2];

    vec4_t v0_world4 =
        mat4_mul_vec4(model_m, vec3_to_vec4(mesh->vertices[index0]));
    vec4_t v1_world4 =
        mat4_mul_vec4(model_m, vec3_to_vec4(mesh->vertices[index1]));
    vec4_t v2_world4 =
        mat4_mul_vec4(model_m, vec3_to_vec4(mesh->vertices[index2]));

    vec3_t v0_world = {v0_world4.x, v0_world4.y, v0_world4.z};
    vec3_t v1_world = {v1_world4.x, v1_world4.y, v1_world4.z};
    vec3_t v2_world = {v2_world4.x, v2_world4.y, v2_world4.z};

    vec3_t triangle_normal =
        calculate_triangle_normal(v0_world, v1_world, v2_world);
    vec3_t center = calculate_triangle_center(v0_world, v1_world, v2_world);
    vec3_t view_ray = vec3_sub(center, g->camera_pos);

    float dot = vec3_dot(triangle_normal, view_ray);
    if (dot > 0)
      continue;

    float alignment = vec3_dot(triangle_normal, g->light_dir);
    if (alignment < 0.0f)
      alignment = 0.0f;

    float final_intensity =
        g->ambient_light + (alignment * (1.0f - g->ambient_light));

    uint32_t final_color = apply_light_intensity(color, final_intensity);

    vec4_t v0_clip = mat4_mul_vec4(g->view_projection, vec3_to_vec4(v0_world));
    vec4_t v1_clip = mat4_mul_vec4(g->view_projection, vec3_to_vec4(v1_world));
    vec4_t v2_clip = mat4_mul_vec4(g->view_projection, vec3_to_vec4(v2_world));

    vec4_t clipped_triangles[2][3];
    int num_triangles =
        near_plane_clipping(v0_clip, v1_clip, v2_clip, clipped_triangles);

    for (int t = 0; t < num_triangles; t++) {
      triangle_t triangle;

      for (int p = 0; p < 3; p++) {
        triangle.points[p].pos =
            clip_to_screen(clipped_triangles[t][p], g->width, g->height);
      }

      draw_filled_triangle(&triangle, final_color, g);

      draw_line(triangle.points[0], triangle.points[1], COLOR_BLACK);
      draw_line(triangle.points[1], triangle.points[2], COLOR_BLACK);
      draw_line(triangle.points[2], triangle.points[0], COLOR_BLACK);
    }
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

void draw_filled_triangle(const triangle_t *triangle, uint32_t color,
                          game_t *g) {
  g->triangle_count++;

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

vec4_t interpolate_at_near_plane(vec4_t v0, vec4_t v1, float znear) {
  float t = (znear - v0.w) / (v1.w - v0.w);

  vec4_t result;
  result.x = v0.x + t * (v1.x - v0.x);
  result.y = v0.y + t * (v1.y - v0.y);
  result.z = v0.z + t * (v1.z - v0.z);
  result.w = znear;
  return result;
}

int near_plane_clipping(vec4_t v0_clip, vec4_t v1_clip, vec4_t v2_clip,
                        vec4_t clipped_triangles[2][3]) {
  float znear = 0.1f;

  bool v0_behind = (v0_clip.w < znear);
  bool v1_behind = (v1_clip.w < znear);
  bool v2_behind = (v2_clip.w < znear);

  int behind_count = v0_behind + v1_behind + v2_behind;

  if (behind_count == 0) {
    clipped_triangles[0][0] = v0_clip;
    clipped_triangles[0][1] = v1_clip;
    clipped_triangles[0][2] = v2_clip;
    return 1;
  }

  if (behind_count == 1) {
    vec4_t behind, front1, front2;

    if (v0_behind) {
      behind = v0_clip;
      front1 = v1_clip;
      front2 = v2_clip;
    } else if (v1_behind) {
      behind = v1_clip;
      front1 = v0_clip;
      front2 = v2_clip;
    } else {
      behind = v2_clip;
      front1 = v0_clip;
      front2 = v1_clip;
    }

    vec4_t inter1 = interpolate_at_near_plane(behind, front1, znear);
    vec4_t inter2 = interpolate_at_near_plane(behind, front2, znear);

    clipped_triangles[0][0] = front1;
    clipped_triangles[0][1] = front2;
    clipped_triangles[0][2] = inter1;

    clipped_triangles[1][0] = inter1;
    clipped_triangles[1][1] = front2;
    clipped_triangles[1][2] = inter2;

    return 2;
  }

  if (behind_count == 2) {
    vec4_t front, behind1, behind2;

    if (!v0_behind) {
      front = v0_clip;
      behind1 = v1_clip;
      behind2 = v2_clip;
    } else if (!v1_behind) {
      front = v1_clip;
      behind1 = v0_clip;
      behind2 = v2_clip;
    } else {
      front = v2_clip;
      behind1 = v0_clip;
      behind2 = v1_clip;
    }

    vec4_t inter1 = interpolate_at_near_plane(behind1, front, znear);
    vec4_t inter2 = interpolate_at_near_plane(behind2, front, znear);

    clipped_triangles[0][0] = front;
    clipped_triangles[0][1] = inter1;
    clipped_triangles[0][2] = inter2;

    return 1;
  }

  return 0;
}

void draw_debug_overlay(game_t *g) {
  int y = 10;
  int spacing = 12;
  char buf[128];

  sprintf(buf, "FPS: %d", (int)(1.0f / g->delta_time));
  draw_text(10, y, buf, COLOR_WHITE);
  y += spacing;

  sprintf(buf, "Frame: %.2f ms", g->delta_time * 1000.0f);
  draw_text(10, y, buf, COLOR_WHITE);
  y += spacing;

  sprintf(buf, "Triangles: %d", g->triangle_count);
  draw_text(10, y, buf, COLOR_WHITE);
  y += spacing;

  sprintf(buf, "Entities: %d", g->entity_count);
  draw_text(10, y, buf, COLOR_WHITE);
  y += spacing;

  sprintf(buf, "Cam: (%.1f, %.1f, %.1f)", g->camera_pos.x, g->camera_pos.y,
          g->camera_pos.z);
  draw_text(10, y, buf, COLOR_WHITE);
  y += spacing;

  sprintf(buf, "Yaw: %.1f  Pitch: %.1f", g->camera_yaw, g->camera_pitch);
  draw_text(10, y, buf, COLOR_WHITE);
}
