# Phase 1: Sprite/Character Rendering System - Development Plan

## Phase 1.0: Testing Infrastructure Setup ✅ COMPLETED

### ✅ CMake Configuration
- [x] Added Google Test via FetchContent
- [x] Configured test executable target
- [x] Set up test discovery with CTest

### ✅ Directory Structure
- [x] Created `tests/` directory
- [x] Created `tests/unit/` for unit tests
- [x] Created `tests/integration/` for integration tests
- [x] Created `tests/assets/` for test assets

### ✅ Main Test File
- [x] Created `tests/main.cpp` with Google Test framework
- [x] Configured test runner and basic setup

### ✅ Test Fixtures
- [x] Created `tests/fixtures/TestFixture.h` with common setup
- [x] Implemented asset mounting and cleanup

### ✅ Unit Tests
- [x] `DataFileTest.cpp` - Tests JSON file I/O functionality
- [x] `DebugWindowTest.cpp` - Tests debug window creation
- [x] `UtilsTest.cpp` - Tests utility functions

### ✅ Integration Tests
- [x] `SpriteSystemTest.cpp` - Tests sprite system integration

### ✅ Test Assets
- [x] Copied `skeleton.json` to `tests/assets/`
- [x] Copied `BODY_skeleton.png` to `tests/assets/`
- [x] Copied `HEAD_chain_armor_helmet.png` to `tests/assets/`

### ✅ Build Scripts
- [x] `make test` - Runs all tests
- [x] `make test-coverage` - Generates coverage report
- [x] `make test-debug` - Runs tests with debug output

**Current Testing Status**: 17 tests, 100% pass, ~2ms execution time

---

## Phase 1.1: Sprite Class Implementation ✅ COMPLETED

### ✅ Core Sprite Class
- [x] `Sprite.h` - Header with transform, rendering, and utility methods
- [x] `Sprite.cpp` - Implementation using Cute Framework's sprite API
- [x] Position, scale, rotation, and visibility support
- [x] Proper Cute Framework integration (`cf_make_easy_sprite_from_png`, `cf_draw_sprite`)

### ✅ Sprite Demo Class
- [x] `SpriteDemo.h` - Header for demo functionality
- [x] `SpriteDemo.cpp` - Implementation with interactive demo
- [x] Multiple sprite instances (body, head, demo sprites)
- [x] Keyboard input handling (arrow keys, space bar)
- [x] Animation system (rotation, pulsing scale, bouncing)
- [x] On-screen demo information display

### ✅ Asset Integration
- [x] Uses existing skeleton assets from `tests/assets/skeleton/`
- [x] Loads `skeleton.json` configuration successfully
- [x] Renders `BODY_skeleton.png` and `HEAD_chain_armor_helmet.png` successfully
- [x] Assets properly copied to build directory for runtime access

### ✅ Main Application Integration
- [x] Integrated `SpriteDemo` into `main.cpp`
- [x] Preserved original `main.cpp` functionality
- [x] Added sprite demo alongside existing debug window

### ✅ Build System
- [x] Updated `CMakeLists.txt` to include new source files
- [x] Both main executable and test executable compile successfully

### ✅ Runtime Success
- [x] Executable runs without crashes
- [x] Asset loading works correctly from build directory
- [x] Sprite demo displays and responds to input
- [x] All sprite transformations (position, scale, rotation) working

**Current Status**: ✅ FULLY IMPLEMENTED AND TESTED
**Demo Available**: Interactive sprite demo with keyboard controls
**Asset Loading**: ✅ Working correctly with proper file paths

---

## Phase 1.2: Character Animation System 📋 IN PROGRESS

### Current Status: Issues Identified
- [x] Basic animation classes implemented (`Animation.h/cpp`, `AnimatedSprite.h/cpp`)
- [x] Animation demo integrated into main application
- [ ] **CRITICAL ISSUE**: Sprite positioning not working correctly - sprites appear in wrong location
- [ ] **CRITICAL ISSUE**: Text positioning needs adjustment for lower-left corner
- [ ] **CRITICAL ISSUE**: Build errors in test files due to missing includes

### Animation Sheet Support (NEW REQUIREMENT)
- [ ] **Directional Sprite Sheets**: Support for 4-direction sprites (UP, LEFT, DOWN, RIGHT)
- [ ] **Sheet Layout**: Each row represents a direction, each column represents animation frames
- [ ] **Automatic Direction Detection**: Parse sprite sheet dimensions to determine frame layout
- [ ] **Direction Switching**: Smooth transitions between directional animations

### Animation Sheet Format Specification
```
Sprite Sheet Layout:
- Width: N frames × frame_width (e.g., 9 frames × 64px = 576px)
- Height: 4 directions × frame_height (e.g., 4 × 256px = 1024px)

Direction Mapping:
- Row 0: UP direction
- Row 1: LEFT direction
- Row 2: DOWN direction
- Row 3: RIGHT direction

Frame Layout:
- Column 0: Frame 0 (idle/start)
- Column 1: Frame 1 (animation frame)
- Column N: Frame N (last animation frame)
```

### Implementation Tasks
- [ ] Fix sprite positioning issues in current implementation
- [ ] Fix text positioning for lower-left corner display
- [ ] Extend Animation class to support directional frame sequences
- [ ] Add direction-aware animation switching
- [ ] Implement automatic sprite sheet parsing for direction detection
- [ ] Add animation state machine for smooth direction transitions
- [ ] Create animation blending between directions

### Technical Requirements
- [ ] Support for variable frame counts per direction
- [ ] Configurable frame durations and timing
- [ ] Direction-aware UV coordinate calculation
- [ ] Animation loop control per direction
- [ ] Performance optimization for multiple animated sprites

### Asset Requirements
- [ ] **Idle Animation Sheets**: Single frame per direction (64×1024px)
- [ ] **Walk Cycle Sheets**: Multiple frames per direction (576×1024px for 9-frame walk)
- [ ] **JSON Configuration**: Define frame counts, timing, and direction mapping
- [ ] **Texture Atlasing**: Efficient memory usage for multiple animation sheets

---

## Phase 1.2.1: Animation Sheet Implementation Details 📋 PLANNED

### Current Animation Implementation Analysis ✅
The existing Animation class already provides a solid foundation for sprite sheets:

#### ✅ What's Already Working
- **Sprite Sheet Properties**: `frameSize` and `sheetSize` properties
- **UV Coordinate Calculation**: `getCurrentFrameUV()` method for frame rendering
- **Frame Management**: `addFrameSequence()` and `addWalkCycleForDirection()` methods
- **Direction Support**: Basic direction enum and frame positioning
- **Current Usage**: AnimationDemo successfully uses sprite sheets for idle and walk cycles

#### 🔧 What Needs Enhancement
- **Direction-Aware Frame Management**: Current system treats each direction as separate frames
- **Automatic Sheet Parsing**: No automatic detection of frame counts per direction
- **Efficient Direction Switching**: Current system recreates animations when direction changes
- **Memory Optimization**: Better organization for directional animation sheets

### Implementation Strategy: Enhance Existing Classes

#### Current Animation Class (Enhanced)
```cpp
class Animation {
    // EXISTING properties (already working)
    std::string name;
    std::vector<AnimationFrame> frames;
    v2 frameSize;
    v2 sheetSize;

    // NEW: Directional frame organization
    std::map<SkeletonDirection, std::vector<AnimationFrame>> directionalFrames;

    // NEW: Automatic sheet parsing
    void parseDirectionalSheet(v2 frameSize, v2 sheetSize);
    v2 getFrameUV(SkeletonDirection direction, int frameIndex);

    // NEW: Direction switching
    void switchDirection(SkeletonDirection newDirection);
    bool hasDirection(SkeletonDirection direction) const;
};
```

### Migration Path: From Current to Enhanced

#### Current Usage (AnimationDemo.cpp)
```cpp
// OLD: Manual setup for each direction
void AnimationDemo::setupAnimations() {
    // Idle animation (single frame for current direction)
    Animation idle_body("idle");
    idle_body.setFrameSize(v2(64, 256));
    idle_body.setSheetSize(v2(64, 256));
    idle_body.addFrame(0, static_cast<int>(currentDirection), 0.5f, "idle_frame");

    // Walk cycle animation for current direction
    Animation walk_body("walk");
    walk_body.setFrameSize(v2(64, 256));
    walk_body.setSheetSize(v2(576, 256)); // 9 frames × 4 directions
    walk_body.addWalkCycleForDirection(currentDirection, 0.1f);
}
```

#### Enhanced Usage (Future)
```cpp
// NEW: Automatic directional sheet parsing
void AnimationDemo::setupAnimations() {
    // Idle animation sheet (automatically parses 4 directions)
    Animation idle_body("idle");
    idle_body.parseDirectionalSheet(v2(64, 256), v2(64, 1024));
    // Automatically detects: 1 frame × 4 directions

    // Walk cycle sheet (automatically parses 9 frames × 4 directions)
    Animation walk_body("walk");
    walk_body.parseDirectionalSheet(v2(64, 256), v2(576, 1024));
    // Automatically detects: 9 frames × 4 directions
}
```

### Technical Implementation Details

#### Enhanced UV Coordinate Calculation
```cpp
v2 Animation::getFrameUV(SkeletonDirection direction, int frameIndex) {
    if (directionalFrames.count(direction) == 0 ||
        frameIndex >= directionalFrames[direction].size()) {
        return v2(0, 0);
    }

    // Calculate UV coordinates for the specific direction and frame
    int directionRow = static_cast<int>(direction);
    float u = (frameIndex * frameSize.x) / sheetSize.x;
    float v = (directionRow * frameSize.y) / sheetSize.y;
    return v2(u, v);
}
```

#### Automatic Directional Sheet Parsing
```cpp
void Animation::parseDirectionalSheet(v2 frameSize, v2 sheetSize) {
    this->frameSize = frameSize;
    this->sheetSize = sheetSize;

    // Calculate frame counts automatically
    int framesPerDirection = static_cast<int>(sheetSize.x / frameSize.x);
    int directionCount = static_cast<int>(sheetSize.y / frameSize.y);

    // Validate direction count (should be 4 for UP, LEFT, DOWN, RIGHT)
    if (directionCount != 4) {
        printf("Warning: Expected 4 directions, got %d\n", directionCount);
    }

    // Create frames for each direction
    for (int dir = 0; dir < directionCount; dir++) {
        SkeletonDirection direction = static_cast<SkeletonDirection>(dir);
        std::vector<AnimationFrame> directionFrames;

        for (int frame = 0; frame < framesPerDirection; frame++) {
            AnimationFrame animFrame(frame, dir, 0.1f,
                "frame_" + std::to_string(frame) + "_dir_" + std::to_string(dir));
            directionFrames.push_back(animFrame);
        }

        directionalFrames[direction] = directionFrames;
    }

    // Set default frames to current direction (DOWN = 2)
    frames = directionalFrames[SkeletonDirection::DOWN];
}
```

### Implementation Priority
1. **Phase 1**: Enhance existing Animation class with directional support
2. **Phase 2**: Update AnimatedSprite to use directional animations
3. **Phase 3**: Modify AnimationDemo to use enhanced system
4. **Phase 4**: Add JSON configuration support
5. **Phase 5**: Performance optimization and testing

### JSON Configuration Format
```json
{
  "animation_sheets": {
    "idle": {
      "file": "assets/skeleton/idle_sheet.png",
      "frame_size": [64, 256],
      "sheet_size": [64, 1024],
      "directions": ["UP", "LEFT", "DOWN", "RIGHT"],
      "frame_count": 1,
      "frame_duration": 0.5
    },
    "walk_cycle": {
      "file": "assets/skeleton/walk_sheet.png",
      "frame_size": [64, 256],
      "sheet_size": [576, 1024],
      "directions": ["UP", "LEFT", "DOWN", "RIGHT"],
      "frame_count": 9,
      "frame_duration": 0.1
    }
  }
}
```

### Testing Strategy
- **Unit Tests**: Test UV coordinate calculation for each direction
- **Integration Tests**: Test direction switching and animation playback
- **Performance Tests**: Measure frame rate with multiple directional sprites
- **Asset Tests**: Validate sprite sheet loading and parsing
- **Migration Tests**: Ensure existing animations continue to work

### Performance Considerations
- **Texture Caching**: Cache loaded animation sheets to avoid repeated loading
- **Frame Precalculation**: Pre-calculate UV coordinates for all frames/directions
- **Batch Rendering**: Group sprites by animation state for efficient rendering
- **Memory Management**: Implement texture atlasing for multiple animation sheets
- **Direction Switching**: Avoid recreating animations, reuse existing frame data

---

## Phase 1.3: SpriteStack Implementation 📋 PLANNED

### Overview
Implement a new `SpriteStack` class that extends the base `Sprite` class to handle directional character sprites with stacked animation frames. This will provide the foundation for the directional animation system.

### Design Goals
- **Extend Base Sprite**: Inherit from `Sprite` for compatibility with existing code
- **Directional Support**: Handle UP, LEFT, DOWN, RIGHT directions
- **Frame Management**: Support multiple frames per direction
- **Easy Integration**: Simple API for direction switching and frame selection

### Implementation Plan
- [ ] **Create SpriteStack.h**: Header with Direction enum and class definition
- [ ] **Create SpriteStack.cpp**: Implementation with UV coordinate calculation
- [ ] **Direction Enum**: UP=0, LEFT=1, DOWN=2, RIGHT=3
- [ ] **Core Methods**: `setDirection()`, `setFrame()`, `getFrameUV()`
- [ ] **Asset Support**: Test with existing skeleton assets
- [ ] **Unit Tests**: Comprehensive testing of all functionality

### Technical Details
- **UV Calculation**: Automatic frame size detection and UV coordinate generation
- **Memory Efficiency**: Single texture load for all directions
- **JSON Configuration**: Frame counts, timing, and direction mapping
- **Performance**: Optimized rendering with minimal overhead

### Integration Points
- **Animation System**: Will use SpriteStack for directional animations
- **Character System**: Will compose multiple SpriteStacks for body parts
- **Existing Code**: Maintains compatibility with current Sprite-based systems

---

## Phase 1.3: Character Layer System 📋 PLANNED

### Planned Features
- [ ] Multi-layer character rendering (body, head, equipment)
- [ ] Layer ordering and depth management
- [ ] Individual layer animation support
- [ ] Equipment attachment system

### Implementation Tasks
- [ ] Create Character class to manage multiple sprites
- [ ] Implement layer management system
- [ ] Add equipment slot system
- [ ] Create layer animation coordination

---

## Testing and Integration ✅ INFRASTRUCTURE READY

### ✅ Unit Testing
- [x] Individual class testing with Google Test
- [x] Mock objects and test fixtures
- [x] Isolated test execution

### ✅ Integration Testing
- [x] Feature-level testing
- [x] Asset loading and rendering tests
- [x] Cross-component interaction tests

### ✅ Test Coverage Goals
- [x] >90% line coverage for core classes
- [x] All public methods tested
- [x] Error handling paths covered

---

## Daily Testing Checklist ✅ READY TO USE

### ✅ Build Verification
- [x] `make build` - Full project compilation
- [x] `make test` - All tests pass
- [x] `make clean` - Clean build verification

### ✅ Runtime Verification
- [x] Main executable runs without crashes
- [x] Sprite demo displays correctly
- [x] Input handling works as expected

---

## Risk Mitigation ✅ ADDRESSED

### ✅ Technical Risks
- [x] **API Mismatch**: Resolved by reading Cute Framework documentation first
- [x] **Build Dependencies**: All dependencies properly configured
- [x] **Asset Loading**: Test assets properly copied and accessible

### ✅ Development Risks
- [x] **Testing Infrastructure**: Comprehensive test suite in place
- [x] **Code Organization**: Clean separation of concerns with SpriteDemo class
- [x] **Documentation**: API usage documented and tested

---

## Current Status Summary

**Phase 1.0**: ✅ COMPLETED - Testing infrastructure fully operational
**Phase 1.1**: ✅ COMPLETED - Sprite class and demo fully implemented
**Phase 1.2**: 🚨 IN PROGRESS - Animation system implemented but has critical positioning issues
**Phase 1.3**: 📋 PLANNED - Character layer system design ready

**Overall Progress**: 2/4 phases completed (50%)
**Testing Status**: ⚠️ Build errors in test files, main executable compiles
**Build Status**: ⚠️ Main executable builds, test executable has compilation errors
**Demo Status**: ⚠️ Animation demo integrated but sprites appear in wrong location

---

## Immediate Next Steps

1. **🚨 CRITICAL FIXES NEEDED**:
   - Fix sprite positioning issues (sprites appearing in upper-right instead of center)
   - Fix text positioning for lower-left corner display
   - Fix test compilation errors due to missing includes

2. **📋 Animation Sheet Implementation**:
   - **Phase 1**: Enhance existing Animation class with directional support (builds on current working foundation)
   - **Phase 2**: Add automatic sprite sheet parsing for direction detection
   - **Phase 3**: Implement direction-aware animation switching system

3. **🧪 Testing and Validation**:
   - Fix test compilation issues
   - Validate sprite positioning fixes
   - Test enhanced directional animation system
   - Ensure existing animations continue to work (backward compatibility)

4. **📋 Performance and Optimization**:
   - Measure rendering performance with multiple animated sprites
   - Implement texture atlasing for animation sheets
   - Optimize direction switching performance

---

## TDD Approach (Ready to Use)

### For New Features
1. **Write Tests First**: Define expected behavior in test cases
2. **Implement Feature**: Write minimal code to pass tests
3. **Refactor**: Clean up code while maintaining test coverage
4. **Repeat**: Continue with next feature

### Current Test Commands
- `make test` - Run all tests
- `make test-debug` - Run tests with verbose output
- `make test-coverage` - Generate coverage report
- `make build` - Build main executable with sprite demo
