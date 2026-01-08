#include <SDL3/SDL.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>

#include "colors.h"
#include "display.h"
#include "math_utils.h"
#include "types.h"

void update_camera_movement(game_t *g) {
  const bool *state = SDL_GetKeyboardState(NULL);
  float move_speed = 10.0f * g->delta_time;

  if (state[SDL_SCANCODE_W]) {
    g->camera_pos =
        vec3_add(g->camera_pos, vec3_mul(g->camera_front, move_speed));
  }

  if (state[SDL_SCANCODE_S]) {
    g->camera_pos =
        vec3_sub(g->camera_pos, vec3_mul(g->camera_front, move_speed));
  }

  if (state[SDL_SCANCODE_A]) {
    g->camera_pos = vec3_sub(
        g->camera_pos,
        vec3_mul(vec3_normalize(vec3_cross(g->camera_front, g->camera_up)),
                 move_speed));
  }

  if (state[SDL_SCANCODE_D]) {
    g->camera_pos = vec3_add(
        g->camera_pos,
        vec3_mul(vec3_normalize(vec3_cross(g->camera_front, g->camera_up)),
                 move_speed));
  }

  if (state[SDL_SCANCODE_SPACE]) {
    g->camera_pos = vec3_add(g->camera_pos, vec3_mul(g->camera_up, move_speed));
  }

  if (state[SDL_SCANCODE_LCTRL]) {
    g->camera_pos = vec3_sub(g->camera_pos, vec3_mul(g->camera_up, move_speed));
  }

  vec3_t direction;
  direction.x =
      cos(deg_to_rad(g->camera_yaw)) * cos(deg_to_rad(g->camera_pitch));
  direction.y = sin(deg_to_rad(g->camera_pitch));
  direction.z =
      sin(deg_to_rad(g->camera_yaw)) * cos(deg_to_rad(g->camera_pitch));

  g->camera_front = vec3_normalize(direction);
}

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

  SDL_SetRenderVSync(g->renderer, 1);

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
  g->cube_pos = (vec3_t){0.0f, 0.0f, -10.0f};

  g->rotation_speed = 0.0f;
  g->total_rotation = 0.0f;

  g->projection_matrix = mat4_make_perspective(
      90.0f, (float)g->width / (float)g->height, 0.1f, 100.0f);

  g->camera_pos = (vec3_t){0.0f, 0.0f, 3.0f};
  g->camera_front = (vec3_t){0.0f, 0.0f, -1.0f};
  g->camera_up = (vec3_t){0.0f, 1.0f, 0.0f};

  g->view_matrix = mat4_look_at(
      g->camera_pos, vec3_add(g->camera_pos, g->camera_front), g->camera_up);

  g->camera_yaw = -90.0f;
  g->camera_pitch = 0.0f;
  g->sensitivity = 0.05f;

  SDL_RaiseWindow(g->window);
  SDL_SetWindowRelativeMouseMode(g->window, true);
  SDL_HideCursor();

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

  g->view_matrix = mat4_look_at(
      g->camera_pos, vec3_add(g->camera_pos, g->camera_front), g->camera_up);

  update_camera_movement(g);
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
      if (event.key.key == SDLK_L) {
        g->rotation_speed += 1.0f;
      }
      if (event.key.key == SDLK_K) {
        g->rotation_speed -= 1.0f;
      }
      break;

    case SDL_EVENT_MOUSE_MOTION:
      g->camera_yaw += event.motion.xrel * g->sensitivity;
      g->camera_pitch -= event.motion.yrel * g->sensitivity;

      if (g->camera_pitch > 89.0f)
        g->camera_pitch = 89.0f;
      if (g->camera_pitch < -89.0f)
        g->camera_pitch = -89.0f;
      break;

    case SDL_EVENT_WINDOW_FOCUS_LOST:
      SDL_SetWindowRelativeMouseMode(g->window, false);
      break;
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
      SDL_SetWindowRelativeMouseMode(g->window, true);
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
