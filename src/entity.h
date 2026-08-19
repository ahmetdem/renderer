#ifndef ENTITY_H
#define ENTITY_H

#include "types.h"

void add_entity_tag(entity_t *entity, entity_tag_t tag);
void remove_entity_tag(entity_t *entity, entity_tag_t tag);
bool has_entity_tag(entity_t *entity, entity_tag_t tag);
void reverse_entity_tag(entity_t *entity, entity_tag_t tag);

entity_t* find_entity(entity_tag_t tag, game_t* g);

#endif
