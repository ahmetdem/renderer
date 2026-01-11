#ifndef GAME_H
#define GAME_H

#include "colors.h"
#include "display.h"
#include "math_utils.h"
#include "types.h"

bool game_init(game_t *g);
void game_update(game_t *g);
void game_render(game_t *g);
void game_exit(game_t *g);

void update_camera_movement(game_t *g);
void handle_input(game_t *g);

#endif
