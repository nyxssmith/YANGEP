# Phase 1: Character Sprite Loading and Rendering

## Overview
Phase 1 focuses on implementing a robust character sprite system using Cute Framework's PNG API. This phase establishes the foundation for loading, managing, and rendering directional character sprites with proper UV coordinate support.

## Research: Cute Framework Asset Loading and Rendering

### 1. Cute Framework PNG API Documentation

#### PNG Cache Functions (Primary Target)
Based on the [Cute Framework API Reference](https://randygaul.github.io/cute_framework/api_reference/#functions_23), the PNG cache provides these key functions:

- **`cf_png_cache_load`** - Loads PNG images from disk into cache
- **`cf_png_cache_load_from_memory`** - Loads PNG images from memory into cache
- **`cf_make_png_cache_sprite`** - Creates sprites from PNG cache data
- **`cf_make_png_cache_animation`** - Creates animations from PNG arrays
- **`cf_make_png_cache_animation_table`** - Creates animation tables for sprites
- **`cf_png_cache_unload`** - Unloads PNG data from cache

#### PNG Cache Structures
According to the [CF_PNG documentation](https://randygaul.github.io/cute_framework/png_cache/cf_png/):

```cpp
typedef struct CF_Png
{
    const char* path;      // Path to PNG file on disk
    uint64_t id;          // Unique identifier assigned by cache
    CF_Pixel* pix;        // Pointer to raw pixel data
    int w;                // Width in pixels
    int h;                // Height in pixels
} CF_Png;
```

#### Key Advantages of PNG API
1. **Direct Pixel Access**: `CF_Png.pix` provides raw RGBA pixel data
2. **Caching System**: Automatic memory management and reuse
3. **Performance**: Optimized loading and memory handling
4. **Flexibility**: Can create custom rendering solutions

### 2. Cute Framework Rendering Architecture

#### Drawing Functions
From the API reference, the drawing system provides:
- **`cf_draw_sprite`** - Standard sprite rendering (limited UV support)
- **`cf_draw_push/pop`** - Transform stack management
- **`cf_draw_translate_v2`** - Position transformations
- **`cf_draw_scale_v2`** - Scale transformations
- **`cf_draw_rotate`** - Rotation transformations

#### Graphics Pipeline
The low-level graphics API (when needed) provides:
- **`cf_make_texture`** - Create textures from pixel data
- **`cf_make_mesh`** - Create geometry for rendering
- **`cf_make_shader`** - Create custom shaders
- **`cf_make_material`** - Bind textures and uniforms

### 3. Asset Loading Strategy

#### PNG Loading Flow
1. **Cache Check**: `cf_png_cache_load()` checks if PNG is already cached
2. **Disk Loading**: If not cached, loads from disk and stores in cache
3. **Pixel Access**: Returns `CF_Png` with direct access to pixel data
4. **Memory Management**: Cache automatically handles memory cleanup

#### Texture Creation
1. **Pixel Data**: Extract RGBA data from `CF_Png.pix`
2. **Texture Parameters**: Use `cf_texture_defaults()` for proper setup
3. **GPU Upload**: `cf_texture_update()` transfers data to GPU
4. **Resource Management**: Proper cleanup with `cf_destroy_texture()`

### 4. Rendering Strategy

#### UV Coordinate Implementation
1. **Frame Calculation**: Calculate UV coordinates for each 64x64 frame
2. **Shader Uniforms**: Pass UV coordinates to custom shaders
3. **Texture Sampling**: Shader samples only the specified UV region
4. **Transform Application**: Apply position, scale, and rotation

#### Performance Considerations
1. **Batch Rendering**: Group similar sprites for efficient GPU calls
2. **Texture Atlases**: Consider combining multiple sprites into single textures
3. **LOD System**: Implement level-of-detail for distance-based rendering
4. **Culling**: Skip rendering of off-screen sprites

## Architecture: PNG API-Based Sprite System

### 1. Core Components

#### SpriteBatch Class
```cpp
class SpriteBatch : public Sprite {
private:
    // PNG cache data
    CF_Png pngData;
    CF_Texture customTexture;

    // Rendering components
    CF_Mesh quadMesh;
    CF_Shader uvShader;
    CF_Material spriteMaterial;

    // Frame management
    int totalFrames;
    int currentFrame;
    Direction currentDirection;
    v2 frameSize;
    v2 renderScale;

public:
    // Core methods
    virtual void render() override;
    void setDirection(Direction direction);
    void setFrame(int frame);

private:
    void initializePNGData();
    void createCustomTexture();
    void setupRenderingPipeline();
    v2 calculateFrameUV(int frame, Direction direction);
};
```

#### Direction Enum
```cpp
enum class Direction {
    UP = 0,      // Top row in sprite sheet
    LEFT = 1,    // Second row in sprite sheet
    DOWN = 2,    // Third row in sprite sheet
    RIGHT = 3    // Bottom row in sprite sheet
};
```

### 2. Data Flow Architecture

#### Loading Phase
```
PNG File → cf_png_cache_load() → CF_Png → Pixel Data → Custom Texture
```

#### Rendering Phase
```
Frame Request → UV Calculation → Shader Uniforms → GPU Rendering → Screen
```

#### Memory Management
```
Cache Management → Automatic Cleanup → Resource Destruction
```

### 3. UV Coordinate System

#### Sprite Sheet Layout
```
┌─────────────────┐
│  UP (0,0)      │ ← Frame 0, Direction UP
├─────────────────┤
│  LEFT (0,0.25) │ ← Frame 0, Direction LEFT
├─────────────────┤
│  DOWN (0,0.5)  │ ← Frame 0, Direction DOWN
├─────────────────┤
│  RIGHT (0,0.75)│ ← Frame 0, Direction RIGHT
└─────────────────┘
```

#### UV Calculation
```cpp
v2 calculateFrameUV(int frameIndex, Direction direction) {
    // For a stacked sprite sheet:
    // - Each row represents a direction (UP=0, LEFT=1, DOWN=2, RIGHT=3)
    // - Since we only have 1 frame per direction, frameIndex should always be 0
    // - The direction determines which row (Y coordinate)

    // Calculate the UV coordinates for the current frame
    // This represents the top-left corner of the frame in UV space
    float u = 0.0f; // Single frame, so always at left edge
    float v = (float)static_cast<int>(direction) / 4.0f; // 4 directions, so each takes 1/4 of height

    return v2(u, v);
}
```

## Implementation Phases

### Phase 1.1: PNG Loading Foundation
- [ ] Implement `cf_png_cache_load()` integration
- [ ] Create `CF_Png` data management
- [ ] Add error handling for failed loads
- [ ] Implement cache cleanup and memory management

### Phase 1.2: Custom Texture System
- [ ] Create texture from PNG pixel data
- [ ] Implement proper texture parameter setup
- [ ] Add texture update and management
- [ ] Implement resource cleanup

### Phase 1.3: Rendering Pipeline
- [ ] Create custom quad mesh for rendering
- [ ] Implement UV-aware shader system
- [ ] Set up material and texture binding
- [ ] Add transform and rendering logic

### Phase 1.4: Frame Management
- [ ] Implement UV coordinate calculations
- [ ] Add direction and frame switching
- [ ] Create frame navigation methods
- [ ] Add render scale and positioning

### Phase 1.5: Integration and Testing
- [ ] Integrate with existing sprite system
- [ ] Add comprehensive error handling
- [ ] Implement performance optimization
- [ ] Add unit and integration tests

## Technical Requirements

### 1. Dependencies
- **Cute Framework**: Core framework and PNG cache API
- **OpenGL/DirectX**: Low-level graphics API (via Cute Framework)
- **PNG Support**: Image format loading and decoding

### 2. Performance Targets
- **Loading Time**: < 100ms for 64x256 sprite sheets
- **Rendering**: 60 FPS with 100+ sprites
- **Memory Usage**: < 10MB for typical sprite collections
- **Cache Hit Rate**: > 90% for frequently used assets

### 3. Quality Standards
- **UV Accuracy**: Pixel-perfect frame boundaries
- **Transform Support**: Position, scale, and rotation
- **Error Handling**: Graceful fallbacks for missing assets
- **Memory Safety**: No memory leaks or corruption

## Success Criteria

### 1. Functional Requirements
- [ ] Individual 64x64 frames render correctly
- [ ] Direction changes update displayed frame
- [ ] UV coordinates are accurately calculated
- [ ] Transform operations work properly
- [ ] Memory management is robust

### 2. Performance Requirements
- [ ] Loading performance meets targets
- [ ] Rendering performance meets targets
- [ ] Memory usage stays within limits
- [ ] Cache efficiency is maintained

### 3. Quality Requirements
- [ ] All tests pass consistently
- [ ] Error handling is comprehensive
- [ ] Code follows Cute Framework patterns
- [ ] Documentation is complete and accurate

## Risk Assessment

### 1. Technical Risks
- **PNG API Complexity**: Mitigation - Thorough documentation review
- **Shader Implementation**: Mitigation - Start with simple vertex/fragment shaders
- **Memory Management**: Mitigation - Implement comprehensive testing
- **Performance Issues**: Mitigation - Profile early and optimize iteratively

### 2. Integration Risks
- **Framework Changes**: Mitigation - Use stable, documented APIs
- **Asset Format Changes**: Mitigation - Implement flexible loading system
- **Rendering Pipeline Changes**: Mitigation - Abstract rendering details

### 3. Timeline Risks
- **Complexity Underestimation**: Mitigation - Break into smaller phases
- **Testing Overhead**: Mitigation - Implement testing alongside development
- **Documentation Lag**: Mitigation - Document as you implement

## Next Steps

1. **Research Completion**: Review all Cute Framework documentation
2. **Architecture Validation**: Confirm approach with framework capabilities
3. **Prototype Development**: Create minimal working example
4. **Implementation Planning**: Break down into manageable tasks
5. **Development Start**: Begin Phase 1.1 implementation

## Detailed Architecture Reference

For comprehensive details on the PNG API architecture, data structures, and implementation patterns, see:

**[Phase 1: PNG API Architecture Guide](phase1_png_api_architecture_guide.md)**

This guide provides:
- **Complete Data Structure Documentation**: CF_PNG, CF_SPRITE, CF_ANIMATION
- **Animation Table Implementation**: How to create and manage animation states
- **UV Coordinate System**: Detailed frame calculation and rendering
- **Memory Management Strategy**: Cache lifecycle and resource cleanup
- **Performance Optimization**: Caching benefits and rendering efficiency
- **Error Handling**: Comprehensive error management and recovery
- **Testing Strategy**: Unit, integration, and visual testing approaches

## Implementation Notes

### Animation Table Strategy
Based on the PNG API research, we'll implement animation tables to manage different sprite states:

```cpp
// Create animations for each direction
const CF_Animation* idle_anim = cf_make_png_cache_animation(
    "idle", &png_data, 1, &idle_delay, 1
);

const CF_Animation* walk_anim = cf_make_png_cache_animation(
    "walk", &png_data, 1, &walk_delay, 1
);

// Create animation table
const CF_Animation* animations[] = {idle_anim, walk_anim};
const htbl CF_Animation** anim_table = cf_make_png_cache_animation_table(
    "skeleton_sprite", animations, 2
);

// Create sprite with animation support
CF_Sprite sprite = cf_make_png_cache_sprite("skeleton_sprite", anim_table);
```

### UV Coordinate Implementation
The UV coordinate system will be implemented using custom shaders that sample only the specified frame region:

```cpp
// Calculate UV coordinates for current frame and direction
v2 frameUV = calculateFrameUV(currentFrame, currentDirection);
float frameWidth = frameSize.x / (float)pngData.w;
float frameHeight = frameSize.y / (float)pngData.h;

// Pass to shader uniforms for proper texture sampling
// Shader will sample only the UV region we specify
```

This approach leverages Cute Framework's PNG API efficiently while providing the UV coordinate support we need for directional sprite rendering.
