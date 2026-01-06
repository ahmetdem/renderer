#include "display.h"
#include "math_utils.h"
#include <SDL3/SDL_stdinc.h>
#include <string.h>

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
      draw_pixel(x + x2, y + y2, calculate_lerp(0xFF0000FF, color, t));
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

        if (x0 == x1 && y0 == y1) break; 

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
