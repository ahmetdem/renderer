#include <SDL3/SDL.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>
#include <stdint.h>
#include <stdio.h>

#include "game.h"
#include "types.h"

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
