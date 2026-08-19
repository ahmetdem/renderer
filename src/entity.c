#include "entity.h"
#include "types.h"
#include <stdbool.h>

bool has_entity_tag(entity_t *entity, entity_tag_t tag) {
  if (!entity)
    return false;

  return (entity->tags & tag) == tag;
}

void add_entity_tag(entity_t *entity, entity_tag_t tag) {
  if (!entity)
    return;
  entity->tags |= tag;
}

void remove_entity_tag(entity_t *entity, entity_tag_t tag) {
  if (!entity)
    return;
  entity->tags &= ~tag;
}

void reverse_entity_tag(entity_t *entity, entity_tag_t tag) {
  if (!entity)
    return;
  entity->tags ^= tag;
}

entity_t *find_entity(entity_tag_t tag, game_t *g) {
  if (!g || !g->entities)
    return NULL;

  for (int i = 0; i < g->entity_count; i++) {
    if (has_entity_tag(&g->entities[i], tag)) {
      return &g->entities[i];
    }
  }

  return NULL;
}
