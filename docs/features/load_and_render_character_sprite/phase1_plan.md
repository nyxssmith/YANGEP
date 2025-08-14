# Phase 1: Core Sprite System - Detailed Development Plan

## Overview
This document breaks down Phase 1 into granular development steps, providing specific implementation details, code structure, and testing checkpoints for each component.

## Phase 1.0: Testing Infrastructure Setup

### Task 1.0.1: Set Up Testing Framework
**Estimated Time**: 2-3 days
**Files**: `CMakeLists.txt`, `tests/`, `scripts/test.sh`

#### Implementation Steps:
1. **Add testing dependencies to CMakeLists.txt**
   - Google Test (gtest) framework
   - Test discovery and execution
   - Coverage reporting tools

2. **Create test directory structure**
   ```
   tests/
   ├── unit/           # Unit tests for individual classes
   ├── integration/    # Integration tests for features
   ├── fixtures/       # Test data and mock objects
   └── main.cpp        # Test runner
   ```

3. **Implement `make test` command**
   - Add test target to CMakeLists.txt
   - Create test.sh script for easy test execution
   - Integrate with existing makefile

4. **Create stub test suite**
   - Basic test framework loading
   - Project dependency verification
   - Sample test for existing classes (DataFile, DebugWindow, Utils)

#### Testing Checkpoint:
- [ ] `make test` command works
- [ ] Test framework loads without errors
- [ ] Project dependencies are accessible in tests
- [ ] Sample tests pass for existing classes
- [ ] Test coverage reporting works

### Task 1.0.2: Set Up Test Data and Fixtures
**Estimated Time**: 1-2 days
**Dependencies**: Task 1.0.1

#### Implementation Steps:
1. **Create test assets**
   - Sample textures for sprite testing
   - Test JSON files for data loading
   - Mock character assets

2. **Set up test fixtures**
   - Base test class with common setup
   - Asset loading helpers
   - Memory leak detection

#### Testing Checkpoint:
- [ ] Test assets load correctly
- [ ] Fixtures provide consistent test environment
- [ ] Memory leak detection works
- [ ] Tests can run in isolation

## Phase 1.1: Sprite Class Implementation

### Task 1.1.1: Create Basic Sprite Class Structure
**Estimated Time**: 1-2 days
**Files**: `src/lib/Sprite.h`, `src/lib/Sprite.cpp`

#### Implementation Steps:
1. **Create Sprite.h header**
   ```cpp
   class Sprite {
   private:
       v2 position;           // Position in world space
       v2 scale;              // Scale factors (x, y)
       float rotation;         // Rotation in radians
       float z_order;         // Z-ordering for depth
       CF_Sprite sprite;      // Cute Framework sprite handle
       bool visible;          // Visibility flag

   public:
       // Constructors
       Sprite();
       Sprite(const char* texture_path);

       // Core methods
       void render();
       void update(float dt);

       // Transform methods
       void setPosition(v2 pos);
       void setScale(v2 scale);
       void setRotation(float rotation);
       void setZOrder(float z);

       // Getters
       v2 getPosition() const;
       v2 getScale() const;
       float getRotation() const;
   };
   ```

2. **Create Sprite.cpp implementation**
   - Implement constructors with proper initialization
   - Add basic transform methods
   - Implement render method using Cute Framework

#### Testing Checkpoint:
- [ ] Sprite compiles without errors
- [ ] Basic sprite creation works
- [ ] Transform methods function correctly
- [ ] No memory leaks in basic operations

### Task 1.1.2: Add Texture Loading Support
**Estimated Time**: 1-2 days
**Dependencies**: Task 1.1.1

#### Implementation Steps:
1. **Extend Sprite class with texture handling**
   ```cpp
   private:
       CF_Texture texture;    // Texture handle
       v2 texture_size;       // Original texture dimensions
       v2 uv_offset;          // UV offset for sprite sheets
       v2 uv_scale;           // UV scale for sprite sheets

   public:
       bool loadTexture(const char* path);
       void setUVCoordinates(v2 offset, v2 scale);
       void setTextureRegion(v2 top_left, v2 size);
   ```

2. **Implement texture loading using Cute Framework**
   - Use `cf_texture_load` for loading
   - Add error handling for failed loads
   - Implement texture region selection for sprite sheets

#### Testing Checkpoint:
- [ ] Textures load correctly from file paths
- [ ] UV coordinate system works for sprite sheets
- [ ] Error handling works for invalid textures
- [ ] Memory is properly managed for textures

### Task 1.1.3: Implement Advanced Transform Operations
**Estimated Time**: 1 day
**Dependencies**: Task 1.1.2

#### Implementation Steps:
1. **Add matrix-based transforms**
   ```cpp
   public:
       void translate(v2 offset);
       void rotate(float angle);
       void scale(v2 factor);
       void setTransform(v2 pos, float rot, v2 scale);

   private:
       void updateTransformMatrix();
       CF_M3x2 transform_matrix;
   ```

2. **Add transform chaining and interpolation**
   - Support for transform interpolation over time
   - Transform hierarchy (parent-child relationships)

#### Testing Checkpoint:
- [ ] Transform operations work correctly
- [ ] Matrix calculations are accurate
- [ ] Interpolation functions smoothly
- [ ] Performance is acceptable for 100+ sprites

## Phase 1.2: Texture Management System

### Task 1.2.1: Create TextureManager Class
**Estimated Time**: 2-3 days
**Files**: `src/lib/TextureManager.h`, `src/lib/TextureManager.cpp`

#### Implementation Steps:
1. **Design TextureManager as singleton**
   ```cpp
   class TextureManager {
   private:
       static TextureManager* instance;
       std::unordered_map<std::string, CF_Texture> textures;
       std::unordered_map<std::string, v2> texture_sizes;

   public:
       static TextureManager* getInstance();

       CF_Texture loadTexture(const char* path);
       CF_Texture getTexture(const char* path);
       v2 getTextureSize(const char* path);
       void unloadTexture(const char* path);
       void unloadAll();
   };
   ```

2. **Implement texture caching and memory management**
   - Reference counting for shared textures
   - Automatic cleanup of unused textures
   - Memory usage monitoring

#### Testing Checkpoint:
- [ ] Singleton pattern works correctly
- [ ] Textures are properly cached
- [ ] Memory management prevents leaks
- [ ] Performance is improved with caching

### Task 1.2.2: Add Texture Atlas Support
**Estimated Time**: 2-3 days
**Dependencies**: Task 1.2.1

#### Implementation Steps:
1. **Create TextureAtlas class**
   ```cpp
   struct AtlasRegion {
       std::string name;
       v2 top_left;
       v2 size;
       v2 pivot;
   };

   class TextureAtlas {
   private:
       CF_Texture atlas_texture;
       std::unordered_map<std::string, AtlasRegion> regions;

   public:
       bool loadAtlas(const char* json_path);
       AtlasRegion getRegion(const char* name);
       CF_Texture getAtlasTexture() const;
   };
   ```

2. **Implement JSON-based atlas loading**
   - Parse atlas configuration files
   - Support for multiple atlas formats
   - Automatic region detection

#### Testing Checkpoint:
- [ ] Atlas loading works from JSON files
- [ ] Region retrieval is accurate
- [ ] Memory usage is optimized
- [ ] Performance is better than individual textures

### Task 1.2.3: Add Texture Compression and Optimization
**Estimated Time**: 1-2 days
**Dependencies**: Task 1.2.2

#### Implementation Steps:
1. **Implement texture compression**
   - Support for different compression formats
   - Automatic format selection based on usage
   - Quality vs. size trade-offs

2. **Add texture streaming for large assets**
   - Progressive loading for large textures
   - Mipmap generation
   - Texture quality scaling

#### Testing Checkpoint:
- [ ] Compression reduces memory usage
- [ ] Quality is acceptable for game use
- [ ] Streaming works for large textures
- [ ] Performance impact is minimal

## Phase 1.3: Basic Rendering Pipeline

### Task 1.3.1: Create Renderer Class
**Estimated Time**: 2-3 days
**Files**: `src/lib/Renderer.h`, `src/lib/Renderer.cpp`

#### Implementation Steps:
1. **Design Renderer class**
   ```cpp
   class Renderer {
   private:
       std::vector<Sprite*> sprites;
       CF_Camera camera;
       v2 viewport_size;

   public:
       void addSprite(Sprite* sprite);
       void removeSprite(Sprite* sprite);
       void renderAll();
       void setCamera(const CF_Camera& cam);
       void setViewport(v2 size);
   };
   ```

2. **Implement sprite batching**
   - Sort sprites by texture and z-order
   - Batch similar sprites together
   - Minimize draw calls

#### Testing Checkpoint:
- [ ] Sprites render in correct order
- [ ] Batching improves performance
- [ ] Camera system works correctly
- [ ] Viewport changes are handled

### Task 1.3.2: Integrate with Main Loop
**Estimated Time**: 1-2 days
**Dependencies**: Task 1.3.1

#### Implementation Steps:
1. **Modify main.cpp**
   ```cpp
   // Add to main.cpp
   Renderer renderer;
   std::vector<Sprite> test_sprites;

   // In main loop
   renderer.renderAll();
   ```

2. **Add debug controls**
   - Use existing DebugWindow for sprite properties
   - Add sprite creation/deletion controls
   - Performance monitoring

#### Testing Checkpoint:
- [ ] Integration works without breaking existing code
- [ ] Debug controls function properly
- [ ] Performance is maintained
- [ ] No memory leaks in main loop

### Task 1.3.3: Add Basic Camera System
**Estimated Time**: 1-2 days
**Dependencies**: Task 1.3.2

#### Implementation Steps:
1. **Implement camera controls**
   ```cpp
   class Camera {
   private:
       v2 position;
       float zoom;
       float rotation;

   public:
       void setPosition(v2 pos);
       void setZoom(float z);
       void setRotation(float rot);
       CF_Camera getCFCamera() const;
   };
   ```

2. **Add camera input handling**
   - Mouse/touch camera movement
   - Zoom controls
   - Camera bounds and constraints

#### Testing Checkpoint:
- [ ] Camera movement is smooth
- [ ] Zoom works correctly
- [ ] Input handling is responsive
- [ ] Performance is maintained during camera operations

## Testing and Integration

### Testing Infrastructure:
- **Unit Tests**: Individual class testing with Google Test
- **Integration Tests**: Feature-level testing with real dependencies
- **Test Execution**: `make test` command for easy testing
- **Coverage**: Automated coverage reporting

### Daily Testing Checklist:
- [ ] Code compiles without warnings
- [ ] All unit tests pass
- [ ] No memory leaks detected
- [ ] Basic functionality works
- [ ] Performance meets targets
- [ ] Integration with existing systems works

### Weekly Milestone Tests:
- [ ] **Week 1**: Testing infrastructure + Basic sprite rendering and transforms
- [ ] **Week 2**: Texture management and atlasing
- [ ] **Week 3**: Rendering pipeline and camera system

### Test Coverage Goals:
- **Unit Tests**: 90%+ coverage for all new classes
- **Integration Tests**: All major feature workflows covered
- **Performance Tests**: Automated benchmarks for sprite rendering

### Performance Benchmarks:
- **Target**: 1000+ sprites at 60 FPS
- **Memory**: < 100MB for 1000 sprites
- **Load Time**: < 1 second for 100 textures

## Risk Mitigation

### Technical Risks:
1. **Cute Framework Integration Issues**
   - **Mitigation**: Start with simple sprite rendering, add complexity gradually
   - **Fallback**: Document any framework limitations early

2. **Memory Management Complexity**
   - **Mitigation**: Implement reference counting from the start
   - **Fallback**: Use smart pointers if manual management becomes complex

3. **Performance Issues**
   - **Mitigation**: Profile early and often
   - **Fallback**: Implement LOD system if needed

### Timeline Risks:
1. **Over-engineering**
   - **Mitigation**: Focus on MVP features first
   - **Fallback**: Defer advanced features to Phase 2

2. **Integration Complexity**
   - **Mitigation**: Test integration points daily
   - **Fallback**: Create isolated test environment

## Success Criteria for Phase 1

### Minimum Viable Product:
- [ ] Single sprites render correctly on screen
- [ ] Basic transforms (position, scale, rotation) work
- [ ] Texture loading and caching functions
- [ ] Simple camera system is functional
- [ ] Integration with main loop works

### Stretch Goals:
- [ ] Sprite batching improves performance by 50%+
- [ ] Texture atlasing reduces memory usage by 30%+
- [ ] Camera system supports smooth zoom and pan
- [ ] Debug interface shows sprite properties and performance metrics

## Next Phase Preparation

### Handoff Requirements:
- [ ] All Phase 1 classes are fully documented
- [ ] Unit tests cover core functionality
- [ ] Performance benchmarks are documented
- [ ] Known limitations are documented
- [ ] Integration points with Phase 2 are clearly defined

### Phase 2 Dependencies:
- [ ] Sprite class supports component-based rendering
- [ ] TextureManager supports multiple texture formats
- [ ] Renderer can handle layered rendering
- [ ] Camera system supports multiple viewports
