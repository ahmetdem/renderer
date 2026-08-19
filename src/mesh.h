#ifndef MESH_H
#define MESH_H

#include "types.h"
#include <stdio.h>
#include <stdlib.h>

static inline mesh_t *create_cube_mesh() {
  mesh_t *cube_mesh = calloc(1, sizeof(mesh_t));
  if (!cube_mesh)
    return NULL;

  cube_mesh->vertex_count = 8;
  cube_mesh->index_count = 36;

  cube_mesh->vertices = calloc(cube_mesh->vertex_count, sizeof(vec3_t));
  if (!cube_mesh->vertices) {
    free(cube_mesh);
    return NULL;
  }

  cube_mesh->indices = calloc(cube_mesh->index_count, sizeof(int));
  if (!cube_mesh->indices) {
    free(cube_mesh->vertices);
    free(cube_mesh);
    return NULL;
  }

  vec3_t cube_vertices[] = {{-1, -1, -1}, {1, -1, -1}, {1, 1, -1}, {-1, 1, -1},
                            {-1, -1, 1},  {1, -1, 1},  {1, 1, 1},  {-1, 1, 1}};
  memcpy(cube_mesh->vertices, cube_vertices, sizeof(cube_vertices));

  int cube_indices[] = {// Front Face (Z+)
                        4, 5, 6, 4, 6, 7,

                        // Right Face (X+)
                        5, 1, 2, 5, 2, 6,

                        // Back Face (Z-)
                        1, 0, 3, 1, 3, 2,

                        // Left Face (X-)
                        0, 4, 7, 0, 7, 3,

                        // Top Face (Y+)
                        7, 6, 2, 7, 2, 3,

                        // Bottom Face (Y-)
                        0, 1, 5, 0, 5, 4};
  memcpy(cube_mesh->indices, cube_indices, sizeof(cube_indices));

  return cube_mesh;
}

#endif
