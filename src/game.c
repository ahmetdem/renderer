#include "game.h"
#include "chunk.h"
#include "colors.h"
#include "display.h"
#include "entity.h"
#include "math_utils.h"
#include "mesh.h"
#include "types.h"
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_timer.h>
#include <stdlib.h>

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

void add_entity(game_t *g, entity_t entity) {
  if (g->entity_count == g->entity_capacity) {
    g->entity_capacity *= 2;
    g->entities = realloc(g->entities, g->entity_capacity * sizeof(entity_t));
  }

  g->entities[g->entity_count] = entity;
  g->entity_count += 1;
}

void init_game_entities(game_t *g) {
  g->entity_count = 0;
  g->entity_capacity = 16;
  g->entities = malloc(g->entity_capacity * sizeof(entity_t));

  mesh_t *cube_mesh = create_cube_mesh();

  entity_t light_source = {cube_mesh,          {-5, 15, 0},  {0, 0, 0},
                           {0.2f, 0.2f, 0.2f}, COLOR_YELLOW, 0};
  add_entity_tag(&light_source, TAG_LIGHT | TAG_COLLIDABLE);
  add_entity(g, light_source);
}

bool game_init(game_t *g) {
  if (!SDL_Init(SDL_INIT_VIDEO))
    return false;

  g->is_running = true;

  g->width = 1920;
  g->height = 1080;

  g->window = SDL_CreateWindow("renderer", g->width, g->height,
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

  g->depth_buffer = (float *)malloc(g->width * g->height * sizeof(float));
  g->last_frame_time = SDL_GetPerformanceCounter();

  g->rotation_speed = 5.0f;
  g->total_rotation = 0.0f;

  g->projection_matrix = mat4_make_perspective(
      110.0f, (float)g->width / (float)g->height, 0.1f, 100.0f);

  g->camera_pos = (vec3_t){0.0f, 0.0f, 10.0f};
  g->camera_front = (vec3_t){0.0f, 0.0f, -1.0f};
  g->camera_up = (vec3_t){0.0f, 1.0f, 0.0f};

  g->view_matrix = mat4_look_at(
      g->camera_pos, vec3_add(g->camera_pos, g->camera_front), g->camera_up);

  g->view_projection = mat4_mul_mat4(g->projection_matrix, g->view_matrix);

  init_game_entities(g);

  g->light_dir = vec3_normalize(find_entity(TAG_LIGHT, g)->position);
  g->ambient_light = 0.1f;

  g->camera_yaw = -90.0f;
  g->camera_pitch = 0.0f;
  g->sensitivity = 0.05f;

  g->show_debug = true;

  g->test_chunk = create_chunk((vec3_t){0, 0, 0});
  generate_flat_terrain(g->test_chunk);
  build_chunk_mesh(g->test_chunk);

  SDL_RaiseWindow(g->window);
  SDL_SetWindowRelativeMouseMode(g->window, true);
  SDL_HideCursor();

  return true;
}

void game_update(game_t *g) {
  uint64_t current_time = SDL_GetPerformanceCounter();
  g->delta_time = (float)(current_time - g->last_frame_time) /
                  (float)SDL_GetPerformanceFrequency();
  g->last_frame_time = current_time;

  g->view_matrix = mat4_look_at(
      g->camera_pos, vec3_add(g->camera_pos, g->camera_front), g->camera_up);
  g->view_projection = mat4_mul_mat4(g->projection_matrix, g->view_matrix);

  update_camera_movement(g);
}

void game_render(game_t *g) {
  SDL_LockTexture(g->texture, NULL, (void **)&g->pixel_buffer, &g->pitch);
  set_render_target(g);
  clear_screen(COLOR_BLACK);

  g->triangle_count = 0;

  for (int i = 0; i < g->entity_count; i++) {
    draw_entity(&g->entities[i], g);
  }

  vec3_t start = g->entities[0].position;
  vec3_t stop = g->test_chunk->world_pos;

  draw_line_3d(start, stop, COLOR_BLUE, g);

  if (g->show_debug) {
    draw_debug_overlay(g);
  }

  if (g->test_chunk && g->test_chunk->mesh) {
    draw_mesh(g->test_chunk->mesh, g->test_chunk->world_pos, (vec3_t){0, 0, 0},
              (vec3_t){1, 1, 1}, COLOR_GREEN, g);
  }

  SDL_UnlockTexture(g->texture);
  SDL_RenderTexture(g->renderer, g->texture, NULL, NULL);
  SDL_RenderPresent(g->renderer);
}

void game_exit(game_t *g) {
  SDL_DestroyWindow(g->window);
  SDL_DestroyRenderer(g->renderer);
  SDL_DestroyTexture(g->texture);

  free(g->depth_buffer);

  for (int i = 0; i < g->entity_count; i++) {
    mesh_t *m = g->entities[i].mesh;
    if (m) {
      free(m->vertices);
      free(m->indices);
      free(m);
      for (int j = i + 1; j < g->entity_count; j++) {
        if (g->entities[j].mesh == m)
          g->entities[j].mesh = NULL;
      }
    }
  }

  free(g->entities);
  free_chunk(g->test_chunk);

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
        g->rotation_speed += 5.0f;
      }
      if (event.key.key == SDLK_K) {
        g->rotation_speed -= 5.0f;
      }
      if (event.key.key == SDLK_F3) {
        g->show_debug = !g->show_debug;
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
