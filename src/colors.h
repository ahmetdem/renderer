#ifndef COLORS_H
#define COLORS_H

/* Format: 0xRRGGBBAA */

#include <stdint.h>
#define COLOR_RED 0xFF0000FF
#define COLOR_GREEN 0x00FF00FF
#define COLOR_BLUE 0x0000FFFF
#define COLOR_WHITE 0xFFFFFFFF
#define COLOR_BLACK 0x000000FF
#define COLOR_YELLOW 0xFFFF00FF
#define COLOR_CYAN 0x00FFFFFF
#define COLOR_MAGENTA 0xFF00FFFF
#define COLOR_GRAY 0x808080FF
#define COLOR_ORANGE 0xFFA500FF
#define COLOR_PURPLE 0x800080FF

#define COLOR_TRANSPARENT 0x00000000

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

static inline uint32_t apply_light_intensity(uint32_t c, float i) {
  uint8_t rC = (c >> 24) & 0xFF;
  uint8_t gC = (c >> 16) & 0xFF;
  uint8_t bC = (c >> 8) & 0xFF;
  uint8_t aC = c & 0xFF;

  // Calculate with floats and clamp to 255
  float rF = (float)rC * i;
  float gF = (float)gC * i;
  float bF = (float)bC * i;

  uint8_t r = (rF > 255.0f) ? 255 : (uint8_t)rF;
  uint8_t g = (gF > 255.0f) ? 255 : (uint8_t)gF;
  uint8_t b = (bF > 255.0f) ? 255 : (uint8_t)bF;

  return (r << 24) | (g << 16) | (b << 8) | aC;
}

#endif
