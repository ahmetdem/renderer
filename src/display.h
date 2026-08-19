#ifndef DISPLAY_H
#define DISPLAY_H

#include "types.h"
#include <stdint.h>

// Interface functions
void set_render_target(game_t *g);
void draw_line_3d(vec3_t p0, vec3_t p1, uint32_t color, game_t *g);
bool project_point(vec3_t world_pos, game_t *g, vec3_t *out_screen_vertex);
void draw_pixel(int x, int y, uint32_t color);
void clear_screen(uint32_t color);
void draw_rect(int x, int y, int w, int h, uint32_t color);
void draw_line(vertex_t v0, vertex_t v1, uint32_t color);
void draw_scanline(int x0, int x1, int y, float w0, float w1, uint32_t color);
void draw_mesh(mesh_t *mesh, vec3_t position, vec3_t rotation, vec3_t scale,
               uint32_t color, game_t *g);
void draw_entity(entity_t *entity, game_t *g);
void draw_filled_triangle(const triangle_t *triangle, uint32_t color,
                          game_t *g);
int near_plane_clipping(vec4_t v0_clip, vec4_t v1_clip, vec4_t v2_clip,
                        vec4_t clipped_triangles[2][3]);
void draw_debug_overlay(game_t *g);

#endif
