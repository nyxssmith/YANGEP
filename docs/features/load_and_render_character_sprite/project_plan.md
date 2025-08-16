# Character Sprite System Project Plan

## Overview
Implement a robust character sprite system in the Cute Framework that can load and render individual frames from sprite sheets, supporting directional sprites (UP, LEFT, DOWN, RIGHT) and multiple animation states (idle, walkcycle).

## Current Status: Virtual PNG Approach Tested and Failed

### Test Results Summary
Our comprehensive testing has revealed the following:

1. ✅ **Sprite Sheet Loading**: Successfully loads both idle (64x256) and walkcycle (576x256) sprite sheets
2. ✅ **Virtual PNG Creation**: Successfully creates virtual PNGs pointing to sub-regions (4 for idle, 36 for walkcycle)
3. ✅ **Animation Table Creation**: Framework accepts virtual PNGs and creates animation tables
4. ❌ **Runtime Rendering**: Fails with `CF_ASSERT(png.path)` when framework tries to render virtual PNGs

### Root Cause Identified
The Cute Framework's PNG cache system (`cf_make_png_cache_animation`, `cf_make_png_cache_sprite`) is fundamentally incompatible with our virtual PNG approach. While it accepts the virtual PNGs during creation, it fails at runtime when trying to access the PNG path and pixel data.

**Key Error**: `CF_ASSERT(png.path)` in `cute_png_cache.cpp:181`

## Implementation Strategy (REVISED - Low-Level Graphics API)

### Phase 1: Low-Level Graphics API Implementation (NEW APPROACH)
Since the PNG cache approach is incompatible, we must pivot to the low-level graphics API:

1. **Load sprite sheets directly** using `cf_png_cache_load`
2. **Create textures** using `cf_make_texture` from the loaded PNG data
3. **Implement custom UV coordinate calculation** for sprite sheet frames
4. **Create custom shaders** for frame clipping and rendering
5. **Build rendering pipeline** using `cf_make_mesh`, `cf_make_material`, `cf_make_shader`

### Phase 2: Sprite Sheet Frame Extraction
1. **Calculate frame UVs** for each direction and frame
2. **Implement frame selection** based on current animation state
3. **Handle animation timing** for multi-frame animations (walkcycle)

### Phase 3: Integration and Demo
1. **Update PNGSprite class** to use low-level graphics resources
2. **Implement proper rendering** with UV clipping
3. **Create working demo** that shows directional sprites
4. **Add input handling** for direction changes

## Technical Architecture

### New PNGSprite Class Structure
```cpp
class PNGSprite {
private:
    // Low-level graphics resources
    CF_Texture spriteSheetTexture;  // Single texture for entire sprite sheet
    CF_Mesh quadMesh;               // Simple quad mesh for rendering
    CF_Material spriteMaterial;     // Material with sprite sheet texture
    CF_Shader uvClippingShader;     // Custom shader for frame clipping
    
    // Frame UV data
    struct FrameUV {
        float u1, v1, u2, v2;  // UV coordinates for frame
    };
    std::map<std::string, FrameUV> frameUVs;  // Animation name -> UV coords
    
    // Current state
    std::string currentAnimation;
    Direction currentDirection;
    int currentFrame;
    
public:
    bool loadAnimations(const char* idle_path, const char* walkcycle_path);
    void render();  // Render current frame with UV clipping
    void update(float dt);
    // ... other methods
};
```

### Shader Requirements
1. **Vertex Shader**: Transform vertices and pass UV coordinates
2. **Fragment Shader**: Sample texture with UV clipping for frame selection

## Testing Strategy

### Unit Tests (✅ Working)
- Sprite loading and validation
- Direction and animation state management
- Basic sprite operations

### Integration Tests (✅ Working)
- End-to-end sprite system functionality
- Memory management and cleanup
- Error handling for invalid assets

### Runtime Tests (❌ Failing - Expected)
- Virtual PNG rendering (fails with CF_ASSERT as expected)
- This confirms our approach is incompatible

## Next Steps

1. **Implement low-level graphics API approach** in PNGSprite class
2. **Create custom UV clipping shaders**
3. **Build working sprite rendering pipeline**
4. **Create functional demo** with directional sprites
5. **Add comprehensive runtime testing**

## Asset Specifications

### Idle Animation Sheet
- **File**: `assets/Art/AnimationsSheets/idle/BODY_skeleton.png`
- **Dimensions**: 64x256 pixels
- **Frame Size**: 64x64 pixels
- **Frames per Direction**: 1
- **Total Frames**: 4 (UP, LEFT, DOWN, RIGHT)

### Walkcycle Animation Sheet
- **File**: `assets/Art/AnimationsSheets/walkcycle/BODY_skeleton.png`
- **Dimensions**: 576x256 pixels
- **Frame Size**: 64x64 pixels
- **Frames per Direction**: 9
- **Total Frames**: 36 (4 directions × 9 frames)

## Success Criteria

1. ✅ **Asset Loading**: Sprite sheets load without errors
2. ✅ **Frame Extraction**: Individual 64x64 frames can be identified
3. ✅ **Direction Support**: UP, LEFT, DOWN, RIGHT directions work
4. ✅ **Animation States**: Idle (1 frame) and walkcycle (9 frames) work
5. ❌ **Rendering**: Individual frames render correctly (requires low-level API)
6. ❌ **Performance**: Efficient rendering without memory leaks
7. ❌ **Error Handling**: Graceful failure for invalid assets

## Conclusion

The virtual PNG approach has been thoroughly tested and confirmed to be incompatible with the Cute Framework. The low-level graphics API approach is the correct path forward and will provide the robust sprite system we need.
