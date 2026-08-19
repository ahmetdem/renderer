#include "chunk.h"
#include "types.h"
#include <stdlib.h>
#include <string.h>

chunk_t *create_chunk(vec3_t pos) {
  chunk_t *chunk = calloc(1, sizeof(chunk_t));
  if (!chunk) return NULL;

  chunk->mesh = calloc(1, sizeof(mesh_t));
  memset(chunk->blocks, 0, sizeof(chunk->blocks));

  chunk->world_pos = pos;
  chunk->dirty = false;

  return chunk;
}

void free_chunk(chunk_t *chunk) {
  if (chunk == NULL) {
    return;
  }

  if (chunk->mesh) {
    if (chunk->mesh->indices)
      free(chunk->mesh->indices);
    if (chunk->mesh->vertices)
      free(chunk->mesh->vertices);
    free(chunk->mesh);
  }

  free(chunk);
}

block_t get_block(chunk_t *chunk, int x, int y, int z) {
  if (x < 0 || x >= 16 || y < 0 || y >= 16 || z < 0 || z >= 16) {
    return BLOCK_AIR;
  }

  return chunk->blocks[x][y][z];
}

void set_block(chunk_t *chunk, block_t block, int x, int y, int z) {
  if (x < 0 || x >= 16 || y < 0 || y >= 16 || z < 0 || z >= 16) {
    return;
  }

  chunk->blocks[x][y][z] = block;
  chunk->dirty = true;
}

static void add_face(mesh_t *mesh, int *v_offset, int *i_offset, vec3_t v0,
                     vec3_t v1, vec3_t v2, vec3_t v3) {
  mesh->vertices[*v_offset + 0] = v0;
  mesh->vertices[*v_offset + 1] = v1;
  mesh->vertices[*v_offset + 2] = v2;
  mesh->vertices[*v_offset + 3] = v3;

  mesh->indices[*i_offset + 0] = *v_offset + 0;
  mesh->indices[*i_offset + 1] = *v_offset + 1;
  mesh->indices[*i_offset + 2] = *v_offset + 2;

  mesh->indices[*i_offset + 3] = *v_offset + 0;
  mesh->indices[*i_offset + 4] = *v_offset + 2;
  mesh->indices[*i_offset + 5] = *v_offset + 3;

  *v_offset += 4;
  *i_offset += 6;
}

void build_chunk_mesh(chunk_t *chunk) {
  if (!chunk || !chunk->mesh) return;

  if (chunk->mesh->vertices) {
    free(chunk->mesh->vertices);
    chunk->mesh->vertices = NULL;
  }
  if (chunk->mesh->indices) {
    free(chunk->mesh->indices);
    chunk->mesh->indices = NULL;
  }

  int total_faces = 0;

  for (int x = 0; x < 16; x++) {
    for (int y = 0; y < 16; y++) {
      for (int z = 0; z < 16; z++) {
        block_t block = get_block(chunk, x, y, z);
        if (block == BLOCK_AIR)
          continue;

        if (get_block(chunk, x + 1, y, z) == BLOCK_AIR)
          total_faces++;
        if (get_block(chunk, x - 1, y, z) == BLOCK_AIR)
          total_faces++;
        if (get_block(chunk, x, y + 1, z) == BLOCK_AIR)
          total_faces++;
        if (get_block(chunk, x, y - 1, z) == BLOCK_AIR)
          total_faces++;
        if (get_block(chunk, x, y, z + 1) == BLOCK_AIR)
          total_faces++;
        if (get_block(chunk, x, y, z - 1) == BLOCK_AIR)
          total_faces++;
      }
    }
  }

  if (total_faces == 0) {
    chunk->mesh->vertex_count = 0;
    chunk->mesh->index_count = 0;
    chunk->dirty = false;
    return;
  }

  chunk->mesh->vertex_count = total_faces * 4;
  chunk->mesh->index_count = total_faces * 6;

  chunk->mesh->vertices = calloc(chunk->mesh->vertex_count, sizeof(vec3_t));
  if (!chunk->mesh->vertices) {
    return;
  }

  chunk->mesh->indices = calloc(chunk->mesh->index_count, sizeof(int));
  if (!chunk->mesh->indices) {
    free(chunk->mesh->vertices);
    chunk->mesh->vertices = NULL;
    return;
  }

  int v_offset = 0;
  int i_offset = 0;

  for (int x = 0; x < 16; x++) {
    for (int y = 0; y < 16; y++) {
      for (int z = 0; z < 16; z++) {
        block_t block = get_block(chunk, x, y, z);
        if (block == BLOCK_AIR)
          continue;

        // Top Face (Y+)
        if (get_block(chunk, x, y + 1, z) == BLOCK_AIR) {
          add_face(chunk->mesh, &v_offset, &i_offset,
                   (vec3_t){x,     y + 1, z    },
                   (vec3_t){x,     y + 1, z + 1},
                   (vec3_t){x + 1, y + 1, z + 1},
                   (vec3_t){x + 1, y + 1, z    });
        }

        // Bottom Face (Y-)
        if (get_block(chunk, x, y - 1, z) == BLOCK_AIR) {
          add_face(chunk->mesh, &v_offset, &i_offset,
                   (vec3_t){x,     y, z + 1},
                   (vec3_t){x,     y, z    },
                   (vec3_t){x + 1, y, z    },
                   (vec3_t){x + 1, y, z + 1});
        }

        // Right Face (X+)
        if (get_block(chunk, x + 1, y, z) == BLOCK_AIR) {
          add_face(chunk->mesh, &v_offset, &i_offset,
                   (vec3_t){x + 1, y,     z    },
                   (vec3_t){x + 1, y + 1, z    },
                   (vec3_t){x + 1, y + 1, z + 1},
                   (vec3_t){x + 1, y,     z + 1});
        }

        // Left Face (X-)
        if (get_block(chunk, x - 1, y, z) == BLOCK_AIR) {
          add_face(chunk->mesh, &v_offset, &i_offset,
                   (vec3_t){x, y,     z + 1},
                   (vec3_t){x, y + 1, z + 1},
                   (vec3_t){x, y + 1, z    },
                   (vec3_t){x, y,     z    });
        }

        // Front Face (Z+)
        if (get_block(chunk, x, y, z + 1) == BLOCK_AIR) {
          add_face(chunk->mesh, &v_offset, &i_offset,
                   (vec3_t){x,     y,     z + 1},
                   (vec3_t){x + 1, y,     z + 1},
                   (vec3_t){x + 1, y + 1, z + 1},
                   (vec3_t){x,     y + 1, z + 1});
        }

        // Back Face (Z-)
        if (get_block(chunk, x, y, z - 1) == BLOCK_AIR) {
          add_face(chunk->mesh, &v_offset, &i_offset,
                   (vec3_t){x + 1, y,     z},
                   (vec3_t){x,     y,     z},
                   (vec3_t){x,     y + 1, z},
                   (vec3_t){x + 1, y + 1, z});
        }
      }
    }
  }

  chunk->dirty = false;
}

void generate_flat_terrain(chunk_t *chunk) {
  if (!chunk) return;

  for (int x = 0; x < 16; x++) {
    for (int z = 0; z < 16; z++) {
      for (int y = 0; y < 16; y++) {
        if (y < 4) {
          chunk->blocks[x][y][z] = BLOCK_STONE;
        } else if (y < 6) {
          chunk->blocks[x][y][z] = BLOCK_DIRT;
        } else if (y == 6) {
          chunk->blocks[x][y][z] = BLOCK_GRASS;
        } else {
          chunk->blocks[x][y][z] = BLOCK_AIR;
        }
      }
    }
  }

  chunk->dirty = true;
}
