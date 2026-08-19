# Renderer — Roadmap

## Completed
- [x] Software rasterizer (pixel buffer, scanline, depth buffer)
- [x] 3D math library (vectors, matrices, projection, look-at)
- [x] Perspective projection & backface culling
- [x] Diffuse + ambient lighting
- [x] FPS camera (WASD + mouse look)
- [x] Mesh system (`mesh_t`, `draw_mesh`)
- [x] Entity system (`entity_t`, `add_entity`, `draw_entity`)
- [x] Entity bitmask tag system (`has_entity_tag`, `add_entity_tag`, `remove_entity_tag`, `toggle_entity_tag`)
- [x] 3D projection helper & debug line rendering (`project_point`, `draw_line_3d`)
- [x] Bitmap font renderer (`draw_char`, `draw_text`)
- [x] Debug overlay (F3 toggle: FPS, frame time, triangles, entities, camera)
- [x] High-precision timing (`SDL_GetPerformanceCounter`)
- [x] **Phase 1: World Foundation**
  - [x] Define `block_t` enum (`BLOCK_AIR`, `BLOCK_DIRT`, `BLOCK_GRASS`, `BLOCK_STONE`)
  - [x] Define `chunk_t` struct (16×16×16 block array + `mesh_t` + world position + dirty flag)
  - [x] Write `build_chunk_mesh()` with hidden face removal
  - [x] Generate flat test world (`generate_flat_terrain`)
  - [x] Render chunks via `draw_mesh()` in `game_render()`

---

## Phase 2: Bug Fixes & Stability
- [x] Fix `M_PI` precision (3.14 → 3.14159265358979f)
- [x] Fix perspective matrix aspect ratio (top/right swap)
- [x] Fix `light_dir` — normalize the direction vector
- [x] Fix `project_point` — stop recomputing `view_projection` every call (pass it in or cache it)
- [x] Fix `draw_scanline` division by zero when `x0 == x1`
- [x] Fix `find_entity` — heap corruption (capacity never grows, wrong sizeof)
- [x] Fix `calculate_lerp` signed overflow on negative color deltas
- [x] Add `#ifndef` include guard to `mesh.h`
- [x] Fix memory leaks — free entity meshes in `game_exit`
- [x] Fix `game_exit` — add `SDL_DestroyTexture(g->texture)`
- [x] Add frame rate cap (use `target_frame_rate` / `SDL_Delay`)
- [x] Add Z-bias to `draw_line` for wireframe depth fighting

## Phase 3: Near-Plane Clipping (Camera Fix)
- [x] Clip triangles against the near plane in clip space (before perspective divide)
- [x] Handle 1-vertex-behind case (split into 2 triangles)
- [x] Handle 2-vertices-behind case (clip to 1 triangle)
- [x] Clean up `draw_mesh` pipeline to render clipped triangles directly

## Phase 4: Textures & Block Colors
- [ ] Add `u, v` fields to `vertex_t`
- [ ] Integrate `stb_image.h` to load texture atlas PNG
- [ ] Implement perspective-correct UV interpolation (interpolate `u/w`, `v/w`, `1/w`)
- [ ] Update `draw_scanline()` to sample texture color per pixel
- [ ] Assign UV coordinates per block face in `build_chunk_mesh()`
- [ ] Define per-block-type face colors/textures (grass top=green, sides=brown, bottom=dirt)

## Phase 5: World Manager & Multi-Chunk
- [ ] Define `BLOCK_SIZE` constant (configurable world scale)
- [ ] Define `world_t` struct — chunk hash map or 2D/3D grid
- [ ] `world_get_chunk(world, cx, cy, cz)` — lookup or create chunk
- [ ] `world_get_block(world, wx, wy, wz)` — global block access across chunks
- [ ] `world_set_block(world, wx, wy, wz, type)` — set block + mark chunk dirty
- [ ] Cross-chunk face culling (check neighbor chunks for hidden faces at chunk borders)
- [ ] Render all loaded chunks in `game_render()`
- [ ] Procedural terrain generation (Perlin/simplex noise for height map)
- [ ] Chunk load/unload based on camera distance

## Phase 6: Player & Physics
- [ ] Create player entity with `TAG_PLAYER`
- [ ] Define `AABB` struct (`vec3_t min, max`)
- [ ] Implement AABB-vs-AABB collision detection
- [ ] Implement AABB-vs-world collision (sweep test against solid blocks)
- [ ] Add gravity (constant downward acceleration)
- [ ] Ground detection & landing (stop falling when on solid block)
- [ ] Jump mechanic (Space when grounded)
- [ ] Bind camera to player entity position (eye offset)
- [ ] Separate horizontal movement (WASD) from vertical (gravity/jump)

## Phase 7: Block Interaction
- [ ] Implement DDA raycasting from camera through world grid
- [ ] Find targeted block + face normal
- [ ] Highlight targeted block (wireframe overlay)
- [ ] Block destruction (left click → set block to AIR, rebuild chunk mesh)
- [ ] Block placement (right click → place block on targeted face)
- [ ] Crosshair overlay at screen center
- [ ] Block selection (number keys or scroll wheel to pick block type)

## Phase 8: Performance (prioritized by impact)

### 🔴 High Impact (fill rate is the #1 bottleneck)
- [ ] Frustum culling — skip chunks entirely outside camera view (`is_chunk_in_frustum`). Free 50-70% savings
- [ ] Configurable internal render resolution — render at e.g. 960×540, upscale to display size. Instant 4× fill rate speedup
- [ ] Optimize `draw_scanline` inner loop with fixed-point integer arithmetic (~2× faster than float)
- [ ] Only rebuild chunk mesh when `dirty` flag is set

### 🟡 Medium Impact
- [ ] Occlusion culling — skip chunks fully hidden behind other chunks
- [ ] Reduce overdraw — sort opaque geometry front-to-back so depth test rejects covered pixels early
- [ ] Greedy meshing — merge adjacent same-type block faces into larger quads (fewer triangles per chunk)
- [ ] Multithreaded chunk mesh building (rebuild meshes on worker threads)

### 🟢 Low Impact (already done or negligible)
- [x] Cache `view_projection` matrix per frame (avoid recomputing per vertex)
- [ ] Object pooling for mesh allocations (avoid `malloc`/`free` per chunk rebuild)
- [ ] SIMD intrinsics for `mat4_mul_vec4` and `draw_scanline` (SSE2/AVX)


## Phase 9: Visual Polish
- [ ] Distance fog (blend pixel color toward sky color based on depth)
- [ ] Sky gradient (top=light blue, horizon=white)
- [ ] Day/night cycle (rotate light direction, change sky colors)
- [ ] Simple ambient occlusion on block vertices
- [ ] Water blocks (transparency, blue tint)
- [ ] Animated textures (water flow, lava)

## Phase 10: UI & Game Systems
- [ ] Pause menu (ESC → show cursor, resume/quit)
- [ ] Main menu (New World / Load World / Quit)
- [ ] Hotbar UI (selected block type display)
- [ ] Inventory screen
- [ ] Health / hunger bar (if survival mode)
- [ ] World save to file (serialize chunks to disk)
- [ ] World load from file

## Phase 11: Audio
- [ ] Integrate SDL_mixer or miniaudio
- [ ] Footstep sounds (play when moving on ground)
- [ ] Block break / place sounds
- [ ] Ambient background music
- [ ] 3D positional audio (attenuate by distance)

## Phase 12: Advanced (Stretch Goals)
- [ ] Entities with AI (basic mob pathfinding)
- [ ] Crafting system
- [ ] Biome generation (different terrain per region)
- [ ] Multiplayer (networking, server/client architecture)
- [ ] Mod support (Lua scripting for custom blocks/items)
