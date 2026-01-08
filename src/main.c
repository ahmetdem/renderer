#include <SDL3/SDL.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>
#include <stdint.h>
#include <stdio.h>

#include "colors.h"
#include "display.h"
#include "math_utils.h"

bool game_init(game_t *g) {
  if (!SDL_Init(SDL_INIT_VIDEO))
    return false;

  g->is_running = true;

  g->width = 640;
  g->height = 480;

  g->window = SDL_CreateWindow("3D Renderer", g->width, g->height,
                               SDL_WINDOW_ALWAYS_ON_TOP);
  if (!g->window)
    return false;

  g->renderer = SDL_CreateRenderer(g->window, NULL);
  if (!g->renderer)
    return false;

  g->texture =
      SDL_CreateTexture(g->renderer, SDL_PIXELFORMAT_RGBA8888,
                        SDL_TEXTUREACCESS_STREAMING, g->width, g->height);
  if (!g->texture)
    return false;

  g->pixel_buffer = NULL;
  g->pitch = 0;

  g->last_frame_time = SDL_GetTicks();

  g->cube_rot = (vec3_t){30.0f, 30.0f, 30.0f};
  g->cube_scale = (vec3_t){1.0f, 1.0f, 1.0f};
  g->cube_pos = (vec3_t){0, 0, -15};

  g->rotation_speed = 30.0f;
  g->total_rotation = 0.0f;

  g->projection_matrix = mat4_make_perspective(
      90.0f, (float)g->width / (float)g->height, 0.1f, 100.0f);

  return true;
}

void game_update(game_t *g) {
  uint64_t current_time = SDL_GetTicks();
  g->delta_time = (current_time - g->last_frame_time) / 1000.0f;
  g->last_frame_time = current_time;

  g->total_rotation += g->rotation_speed * g->delta_time;
  g->cube_rot.x = g->total_rotation;
  g->cube_rot.y = g->total_rotation;
  g->cube_rot.z = g->total_rotation;
}

void game_render(game_t *g) {
  SDL_LockTexture(g->texture, NULL, (void **)&g->pixel_buffer, &g->pitch);
  set_render_target(g->pixel_buffer, g->width, g->height, g->pitch);

  clear_screen(COLOR_BLACK);

  draw_cube(g->cube_pos, g->cube_rot, g->cube_scale, g);

  SDL_UnlockTexture(g->texture);
  SDL_RenderTexture(g->renderer, g->texture, NULL, NULL);
  SDL_RenderPresent(g->renderer);
}

void game_exit(game_t *g) {
  SDL_DestroyWindow(g->window);
  SDL_DestroyRenderer(g->renderer);

  SDL_Quit();
}

void handle_input(game_t *g) {
  SDL_Event event;

  while (SDL_PollEvent(&event)) {
    switch (event.type) {
    case SDL_EVENT_QUIT:
      g->is_running = false;
      break;

    case SDL_EVENT_KEY_DOWN:
      if (event.key.key == SDLK_ESCAPE) {
        g->is_running = false;
      }
      if (event.key.key == SDLK_W) {
        g->rotation_speed += 1.0f;
      }
      if (event.key.key == SDLK_S) {
        g->rotation_speed -= 1.0f;
      }

      break;
    }
  }
}

int main() {
  game_t game = {0};

  if (!game_init(&game)) {
    fprintf(stderr, "Failed to initialize game\n");
    return 1;
  }

  while (game.is_running) {
    handle_input(&game);

    game_update(&game);
    game_render(&game);
  }

  game_exit(&game);
  return 0;
}
