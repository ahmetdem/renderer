#ifndef CHUNK_H
#define CHUNK_H

#include "types.h"
#include <stdbool.h>

chunk_t *create_chunk(vec3_t pos);
void free_chunk(chunk_t *chunk);
block_t get_block(chunk_t *chunk, int x, int y, int z);
void set_block(chunk_t *chunk, block_t block, int x, int y, int z);
void build_chunk_mesh(chunk_t *chunk);
void generate_flat_terrain(chunk_t *chunk);

#endif
