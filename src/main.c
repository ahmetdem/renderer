#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>
#include <stdint.h>
#include <stdio.h>

#include "colors.h"
#include "display.h"
#include "math_utils.h"

vec3_t cube_vertices[] = {{-1, -1, -1}, {1, -1, -1}, {1, 1, -1}, {-1, 1, -1},
                          {-1, -1, 1},  {1, -1, 1},  {1, 1, 1},  {-1, 1, 1}};

int main() {
  SDL_Window *window;
  SDL_Renderer *renderer = NULL;
  SDL_Texture *texture = NULL;

  uint64_t previous_frame_time = SDL_GetTicks();
  float rotation_speed = 1.0f;
  static float total_rotation = 0.0f;

  void *pixel_ptr = NULL;
  int pitch = 0;

  int width = 640;
  int height = 480;

  bool done = false;
  SDL_Init(SDL_INIT_VIDEO);

  window =
      SDL_CreateWindow("3D Renderer", width, height, SDL_WINDOW_ALWAYS_ON_TOP);
  renderer = SDL_CreateRenderer(window, NULL);
  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                              SDL_TEXTUREACCESS_STREAMING, width, height);

  if (window == NULL) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create window: %s\n",
                 SDL_GetError());
    return 1;
  }

  while (!done) {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT) {
        done = true;
      }
    }

    uint64_t current_time = SDL_GetTicks();
    float delta_time = (current_time - previous_frame_time) / 1000.0f;
    previous_frame_time = current_time;

    SDL_LockTexture(texture, NULL, &pixel_ptr, &pitch);
    uint32_t *pixels = (uint32_t *)pixel_ptr;
    set_render_target(pixels, width, height, pitch);
    clear_screen(COLOR_BLACK);

    total_rotation += rotation_speed * delta_time;

    mat4_t rotation_z = mat4_make_rotation_z(total_rotation);
    mat4_t rotation_y = mat4_make_rotation_y(total_rotation * 0.5f);
    mat4_t rotation_x = mat4_make_rotation_x(total_rotation * 0.2f);

    mat4_t rotation_m =
        mat4_mul_mat4(rotation_x, mat4_mul_mat4(rotation_y, rotation_z));

    mat4_t translation_m = mat4_make_translation(0, 0, -15);
    mat4_t world_m = mat4_mul_mat4(translation_m, rotation_m);

    mat4_t projection_m = mat4_make_perspective(
        90.0f, (float)width / (float)height, 0.1f, 100.0f);

    vec3_t projected_points[8];

    for (int i = 0; i < 8; i++) {
      vec4_t world_pos = mat4_mul_vec4(world_m, vec3_to_vec4(cube_vertices[i]));

      if (world_pos.z > -0.1f) {
        projected_points[i] = (vec3_t){-1000, -1000, 0};
        continue;
      }

      vec3_t projected = vec3_project(
          (vec3_t){world_pos.x, world_pos.y, world_pos.z}, projection_m);

      projected_points[i].x = (projected.x + 1) * 0.5 * width;
      projected_points[i].y = (1 - projected.y) * 0.5 * height;
    }

    // Front Face
    draw_line(projected_points[0].x, projected_points[0].y,
              projected_points[1].x, projected_points[1].y, COLOR_WHITE);
    draw_line(projected_points[1].x, projected_points[1].y,
              projected_points[2].x, projected_points[2].y, COLOR_WHITE);
    draw_line(projected_points[2].x, projected_points[2].y,
              projected_points[3].x, projected_points[3].y, COLOR_WHITE);
    draw_line(projected_points[3].x, projected_points[3].y,
              projected_points[0].x, projected_points[0].y, COLOR_WHITE);

    // Back Face
    draw_line(projected_points[4].x, projected_points[4].y,
              projected_points[5].x, projected_points[5].y, COLOR_WHITE);
    draw_line(projected_points[5].x, projected_points[5].y,
              projected_points[6].x, projected_points[6].y, COLOR_WHITE);
    draw_line(projected_points[6].x, projected_points[6].y,
              projected_points[7].x, projected_points[7].y, COLOR_WHITE);
    draw_line(projected_points[7].x, projected_points[7].y,
              projected_points[4].x, projected_points[4].y, COLOR_WHITE);

    // Connecting the two faces
    draw_line(projected_points[0].x, projected_points[0].y,
              projected_points[4].x, projected_points[4].y, COLOR_WHITE);
    draw_line(projected_points[1].x, projected_points[1].y,
              projected_points[5].x, projected_points[5].y, COLOR_WHITE);
    draw_line(projected_points[2].x, projected_points[2].y,
              projected_points[6].x, projected_points[6].y, COLOR_WHITE);
    draw_line(projected_points[3].x, projected_points[3].y,
              projected_points[7].x, projected_points[7].y, COLOR_WHITE);

    SDL_UnlockTexture(texture);
    SDL_RenderTexture(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);

    // Game Logic Here
  }

  SDL_DestroyWindow(window);
  SDL_DestroyRenderer(renderer);

  SDL_Quit();
  return 0;
}
