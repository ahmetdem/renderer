#ifndef DISPLAY_H
#define DISPLAY_H

#include "types.h"
#include <stdint.h>

// Interface functions
void set_render_target(game_t *g);
void draw_pixel(int x, int y, uint32_t color);
void clear_screen(uint32_t color);
void draw_rect(int x, int y, int w, int h, uint32_t color);
void draw_line(vertex_t v0, vertex_t v1, uint32_t color);
void draw_scanline(int x0, int x1, int y, float w0, float w1, uint32_t color);
void draw_cube(vec3_t transform, vec3_t rotation, vec3_t scale, game_t *g);
void draw_filled_triangle(const triangle_t *triangle, uint32_t color);

#endif
