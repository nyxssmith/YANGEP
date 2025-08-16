# Sprite/Character Rendering System - Project Plan

## Overview
Implement a comprehensive sprite and character rendering system for YANGEP that can handle static sprites, animated sprites, and modular character components (head, body, etc.) based on the existing art assets.

## Current Assets Analysis
The project already has character art assets organized as:
- **AnimationsSheets/idle/** - Static/idle character poses
  - `HEAD_chain_armor_helmet.png` (2.7KB)
  - `BODY_skeleton.png` (7.8KB)
- **AnimationsSheets/walkcycle/** - Walking animation frames
  - `BODY_skeleton.png` (23KB) - Larger file suggests multiple animation frames
  - `HEAD_plate_armor_helmet.png` (5.4KB)

## Phase 1: Core Sprite System (Priority: High)

### 1.1 Sprite Class Implementation
- **File**: `src/lib/Sprite.h` and `src/lib/Sprite.cpp`
- **Features**:
  - Basic sprite rendering with position, scale, rotation
  - Texture loading and management
  - UV coordinate handling for sprite sheets
  - Z-ordering support
  - Basic transform operations (translate, rotate, scale)

### 1.2 SpriteStack Class Implementation (NEW)
- **File**: `src/lib/SpriteStack.h` and `src/lib/SpriteStack.cpp`
- **Purpose**: Handle directional character sprites with stacked animation frames
- **Features**:
  - **Directional Support**: UP, LEFT, DOWN, RIGHT enumerated states
  - **Stacked Frame Layout**: Long vertical images with frames stacked by direction
  - **Direction Switching**: Methods to change current direction and update rendering
  - **UV Coordinate Management**: Automatic calculation for each direction's frame
  - **Inheritance**: Extends base `Sprite` class for compatibility
  - **Frame Selection**: Methods to select specific direction and frame combination

#### SpriteStack Design
```cpp
enum class Direction {
    UP = 0,
    LEFT = 1,
    DOWN = 2,
    RIGHT = 3
};

class SpriteStack : public Sprite {
    Direction currentDirection;
    int frameCount;
    v2 frameSize;

    void setDirection(Direction dir);
    void setFrame(int frameIndex);
    v2 getFrameUV(int frameIndex, Direction dir);
};
```

#### Asset Layout Support
- **Vertical Stack Layout**: Each row represents a direction (UP=0, LEFT=1, DOWN=2, RIGHT=3)
- **Frame Dimensions**: Automatic detection of frame size and count
- **JSON Configuration**: Define frame counts, timing, and direction mapping per asset
- **Memory Efficiency**: Single texture load for all directions

### 1.3 Texture Management
- **File**: `src/lib/TextureManager.h` and `src/lib/TextureManager.cpp`
- **Features**:
  - Texture loading and caching
  - Memory management for textures
  - Support for PNG, JPG formats
  - Texture atlasing for performance

### 1.3 Basic Rendering Pipeline
- **Integration**: Extend `main.cpp` and existing rendering system
- **Features**:
  - Sprite batching for performance
  - Basic camera/viewport system
  - Render order management

## Phase 2: Character System (Priority: High)

### 2.1 Character Component System
- **File**: `src/lib/Character.h` and `src/lib/Character.cpp`
- **Features**:
  - Modular character assembly (head, body, accessories)
  - Component layering and ordering
  - Character state management (idle, walking, etc.)

### 2.2 Character Renderer
- **File**: `src/lib/CharacterRenderer.h` and `src/lib/CharacterRenderer.cpp`
- **Features**:
  - Multi-component rendering
  - Component positioning and alignment
  - Character customization system

## Phase 3: Animation System (Priority: Medium)

### 3.1 Animation Framework
- **File**: `src/lib/Animation.h` and `src/lib/Animation.cpp`
- **Features**:
  - Frame-based animation system
  - Animation state machine
  - Timeline and keyframe support
  - Animation blending

### 3.2 Sprite Sheet Parser
- **File**: `src/lib/SpriteSheet.h` and `src/lib/SpriteSheet.cpp`
- **Features**:
  - Automatic frame detection from sprite sheets
  - Frame metadata (duration, events, etc.)
  - Support for irregular frame layouts

### 3.3 Directional Animation Sheets (NEW REQUIREMENT)
- **File**: Enhanced `src/lib/Animation.h` and `src/lib/AnimatedSprite.h`
- **Features**:
  - **4-Direction Support**: UP, LEFT, DOWN, RIGHT directional sprites
  - **Automatic Sheet Parsing**: Parse sprite sheet dimensions for direction detection
  - **Direction Switching**: Smooth transitions between directional animations
  - **UV Coordinate Management**: Automatic calculation for each direction/frame
  - **JSON Configuration**: Define sheet layouts, frame counts, and timing per direction

#### Directional Sheet Layout
```
Standard Layout:
- Width: N frames × frame_width (e.g., 9 frames × 64px = 576px)
- Height: 4 directions × frame_height (e.g., 4 × 256px = 1024px)
- Row 0: UP direction, Row 1: LEFT, Row 2: DOWN, Row 3: RIGHT
- Each row contains all animation frames for that direction
```

#### Asset Requirements
- **Idle Sheets**: Single frame per direction (64×1024px)
- **Walk Cycle Sheets**: Multiple frames per direction (576×1024px for 9-frame walk)
- **JSON Configuration**: Define frame counts, timing, and direction mapping
- **Texture Atlasing**: Efficient memory usage for multiple animation sheets

## Phase 4: Advanced Features (Priority: Low)

### 4.1 Performance Optimizations
- **Features**:
  - Sprite culling and frustum testing
  - Level-of-detail system
  - GPU instancing for similar sprites

### 4.2 Visual Effects
- **Features**:
  - Particle systems for character effects
  - Shader support for custom effects
  - Post-processing effects

## Technical Implementation Details

### Dependencies
- **Cute Framework**: For rendering, texture loading, and window management
- **nlohmann/json**: For animation data and character configuration files
- **stb_image**: For image loading (may need to add)

### File Structure
```
src/lib/
├── Sprite.h/cpp              # Basic sprite functionality
├── SpriteStack.h/cpp         # Directional sprite stack support (NEW)
├── TextureManager.h/cpp      # Texture management
├── Character.h/cpp           # Character component system
├── CharacterRenderer.h/cpp   # Character rendering
├── Animation.h/cpp           # Animation framework
├── SpriteSheet.h/cpp         # Sprite sheet parsing
└── Renderer.h/cpp            # Main rendering pipeline
```

### Configuration Files
```
assets/
├── characters/
│   ├── skeleton.json         # Character definition
│   └── animations.json       # Animation data
└── textures/
    └── atlas.json            # Texture atlas configuration
```

## Testing Strategy

### Unit Tests
- Sprite creation and manipulation
- **SpriteStack directional support and frame selection**
- Texture loading and caching
- Animation frame calculation
- Character component assembly

### Integration Tests
- End-to-end character rendering
- Animation playback
- Performance benchmarks
- Memory usage monitoring

### Manual Testing
- Visual verification of character rendering
- Animation smoothness
- Performance on different hardware
- Cross-platform compatibility

## Success Criteria

### Phase 1 Success
- [ ] Basic sprites render correctly on screen
- [ ] Texture loading works without memory leaks
- [ ] Sprite transforms (position, scale, rotation) function properly
- [ ] **SpriteStack Support**: Directional sprites (UP, LEFT, DOWN, RIGHT) render correctly
- [ ] **Direction Switching**: SpriteStack direction changes update rendering immediately
- [ ] **Frame Selection**: Individual frames can be selected from directional stacks
- [ ] Performance: 1000+ sprites at 60 FPS

### Phase 2 Success
- [ ] Character components render in correct layers
- [ ] Character customization system works
- [ ] Character state changes update rendering
- [ ] Performance: 100+ characters at 60 FPS

### Phase 3 Success
- [ ] Animations play smoothly
- [ ] Animation transitions work correctly
- [ ] Sprite sheet parsing is accurate
- [ ] **Directional Animation Support**: 4-direction sprites work correctly (UP, LEFT, DOWN, RIGHT)
- [ ] **Automatic Sheet Parsing**: Sprite sheet dimensions automatically detected and parsed
- [ ] **Direction Switching**: Smooth transitions between directional animations
- [ ] **UV Coordinate Accuracy**: Correct frame rendering for each direction and frame
- [ ] Performance: 50+ animated characters at 60 FPS

## Risk Assessment

### High Risk
- **Performance**: Complex character rendering may impact frame rate
- **Memory**: Large texture atlases could consume significant memory
- **Complexity**: Animation system may become overly complex

### Mitigation Strategies
- **Performance**: Implement sprite batching and culling early
- **Memory**: Add texture compression and streaming
- **Complexity**: Start with simple frame-based animations, add complexity gradually

## Timeline Estimate

- **Phase 1**: 3-4 weeks (increased for SpriteStack implementation)
- **Phase 2**: 2-3 weeks
- **Phase 3**: 4-5 weeks (increased due to directional animation sheet complexity)
- **Phase 4**: 2-3 weeks
- **Total**: 11-15 weeks (increased by 1 week for SpriteStack, 1 week for directional animation sheets)

## Next Steps

1. **Immediate (Critical Fixes)**:
   - Fix sprite positioning issues in current implementation
   - Fix text positioning for lower-left corner display
   - Fix test compilation errors due to missing includes

2. **Week 1**: Implement SpriteStack class for directional sprites
   - Create `SpriteStack.h/cpp` extending base `Sprite` class
   - Implement Direction enum (UP, LEFT, DOWN, RIGHT)
   - Add direction switching and frame selection methods
   - Create unit tests for SpriteStack functionality

3. **Week 2**: Integrate SpriteStack with existing assets
   - Test with current skeleton assets (idle and walkcycle)
   - Implement automatic frame size detection
   - Add JSON configuration support for frame counts and timing
   - Create integration tests with real assets

4. **Week 3**: Begin character component system design
   - Design multi-layer character rendering system
   - Plan equipment attachment system
   - Create layer animation coordination

5. **Week 4**: Implement character rendering pipeline
   - Build character component system
   - Integrate with SpriteStack system
   - Add performance optimization and testing

## Dependencies on Existing Code

- **DataFile system**: For loading character and animation configurations
- **DebugWindow**: For debugging sprite properties and animation states
- **Utils**: For file system operations and JSON parsing
- **Main loop**: For integration with existing rendering pipeline
