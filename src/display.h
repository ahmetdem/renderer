#ifndef DISPLAY_H
#define DISPLAY_H

#include "types.h"
#include <stdint.h>

static inline uint32_t calculate_lerp(uint32_t ac, uint32_t bc, float t) {
  uint8_t rA = (ac >> 24) & 0xFF;
  uint8_t gA = (ac >> 16) & 0xFF;
  uint8_t bA = (ac >> 8) & 0xFF;
  uint8_t aA = ac & 0xFF;

  uint8_t rB = (bc >> 24) & 0xFF;
  uint8_t gB = (bc >> 16) & 0xFF;
  uint8_t bB = (bc >> 8) & 0xFF;
  uint8_t aB = bc & 0xFF;

  uint8_t r = rA + (uint8_t)((rB - rA) * t);
  uint8_t g = gA + (uint8_t)((gB - gA) * t);
  uint8_t b = bA + (uint8_t)((bB - bA) * t);
  uint8_t alpha = aA + (uint8_t)((aB - aA) * t);

  return (r << 24) | (g << 16) | (b << 8) | alpha;
}

// Interface functions
void set_render_target(uint32_t *pixels, int width, int height, int pitch);
void draw_pixel(int x, int y, uint32_t color);
void clear_screen(uint32_t color);
void draw_rect(int x, int y, int w, int h, uint32_t color);
void draw_line(int x0, int y0, int x1, int y1, uint32_t color);
void draw_cube(vec3_t transform, vec3_t rotation, vec3_t scale, game_t *g);

#endif
