# Renderer Development Roadmap

## Phase 1: 3D Foundation (Depth & Solidity)
- [ ] Implement `apply_light_intensity` helper to scale color values.
- [ ] Calculate lighting intensity using dot product of `face_normal` and `light_direction`.
- [ ] Apply calculated lighting color to faces in `draw_cube` (Flat Shading).

## Phase 2: Texture Pipeline
- [ ] Add `u` and `v` coordinate fields to the `vertex_t` struct.
- [ ] Integrate `stb_image.h` or write a parser to load raw pixel data into a `texture_t` struct.
- [ ] Update `triangle_t` to include a pointer to the texture data.
- [ ] Implement perspective-correct interpolation math (interpolate `u/w`, `v/w`, and `1/w`).
- [ ] Update `draw_scanline` to calculate final `u,v` and sample the texture color for each pixel.
- [ ] Add a visual test case rendering a single textured quad.

## Phase 3: World Generation & Performance
- [ ] Define `Block` struct and a `Chunk` struct (array of 16x16x16 blocks).
- [ ] Create a `generate_chunk_mesh` function that builds a single vertex list for an entire chunk.
- [ ] Implement "Hidden Face Removal" to discard faces between touching opaque blocks.
- [ ] Update `game_render` to loop through a list of Chunks instead of a single cube.
- [ ] Implement `is_box_in_frustum` check to skip rendering chunks behind the camera.

## Phase 4: Gameplay & UI
- [ ] Define `AABB` (Axis-Aligned Bounding Box) struct for player and blocks.
- [ ] Implement `check_aabb_collision` function for simple box-vs-box physics.
- [ ] Add gravity and basic ground collision to the camera/player movement.
- [ ] Integrate `cimgui` (or similar) to render a debug window overlay.
- [ ] Create debug sliders for rotation speed, light direction, and clear color.
- [ ] Implement a bitmap font renderer to draw text strings to the pixel buffer.
- [ ] Draw an FPS counter and crosshair on the HUD.

## Phase 5: Polish & Optimization
- [ ] Implement distance fog (blend pixel color with clear color based on depth).
- [ ] Add simple texture transparency support (discard pixels with alpha < threshold).
- [ ] Optimize `draw_scanline` using fixed-point arithmetic to remove float casts.
- [ ] Implement basic frustum clipping (clip triangles that cross the near plane).
