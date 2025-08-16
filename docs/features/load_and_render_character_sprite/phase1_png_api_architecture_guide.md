# Phase 1: PNG API Architecture Guide

## Overview
This document provides comprehensive research on Cute Framework's PNG API architecture, focusing on the data structures and functions needed to implement sprite loading, animation tables, and UV coordinate rendering from PNG assets.

## Core Data Structures Research

### 1. CF_PNG Structure

#### Definition
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

#### Key Characteristics
- **`path`**: String pointer to file location (read-only)
- **`id`**: 64-bit unique identifier for cache management
- **`pix`**: Raw RGBA pixel data pointer
- **`w`**: Image width in pixels
- **`h`**: Image height in pixels

#### Memory Layout
```
CF_Png Structure:
┌─────────────┬─────────────┬─────────────┬─────────────┬─────────────┐
│   path     │     id      │    pix      │     w       │     h       │
│  (char*)   │  (uint64_t) │ (CF_Pixel*) │   (int)     │   (int)     │
└─────────────┴─────────────┴─────────────┴─────────────┴─────────────┘
```

#### CF_Pixel Structure
```cpp
typedef struct CF_Pixel
{
    uint8_t r;    // Red component (0-255)
    uint8_t g;    // Green component (0-255)
    uint8_t b;    // Blue component (0-255)
    uint8_t a;    // Alpha component (0-255)
} CF_Pixel;
```

### 2. CF_Sprite Structure

#### Definition
```cpp
typedef struct CF_Sprite
{
    uint64_t id;          // Unique sprite identifier
    // Additional internal data managed by Cute Framework
} CF_Sprite;
```

#### Key Characteristics
- **`id`**: 64-bit unique identifier
- **Opaque Handle**: Internal structure managed by framework
- **Reference Counting**: Automatic memory management
- **Animation Support**: Can contain multiple animation states

#### Usage Patterns
```cpp
// Create sprite from PNG cache
CF_Sprite sprite = cf_make_png_cache_sprite("sprite_name", animation_table);

// Access sprite properties
int width = cf_sprite_width(&sprite);
int height = cf_sprite_height(&sprite);

// Update sprite state
cf_sprite_update(&sprite, delta_time);
```

### 3. CF_Animation Structure

#### Definition
```cpp
typedef struct CF_Animation
{
    const char* name;         // Animation name/identifier
    CF_Frame* frames;         // Array of animation frames
    int frame_count;          // Number of frames in animation
    float* delays;            // Frame delay array (milliseconds)
    bool looping;             // Whether animation loops
    // Additional internal data
} CF_Animation;
```

#### Key Characteristics
- **`name`**: String identifier for the animation
- **`frames`**: Array of frame data structures
- **`frame_count`**: Total number of frames
- **`delays`**: Timing information for each frame
- **`looping`**: Loop behavior flag

#### CF_Frame Structure
```cpp
typedef struct CF_Frame
{
    CF_Png* png;              // Reference to PNG data
    v2 offset;                // Position offset within frame
    v2 size;                  // Frame dimensions
    float delay;              // Frame duration (milliseconds)
} CF_Frame;
```

### 4. Animation Table Structure

#### Definition
```cpp
typedef const htbl CF_Animation** CF_AnimationTable;
```

#### Key Characteristics
- **Hash Table**: Maps animation names to CF_Animation pointers
- **Multiple Animations**: Can contain several named animations
- **Lookup Performance**: O(1) animation retrieval by name
- **Memory Management**: Framework handles cleanup

#### Structure Layout
```
Animation Table:
┌─────────────────────────────────────────────────────────────┐
│ Hash Table (htbl)                                          │
├─────────────────────────────────────────────────────────────┤
│ Key: "idle"     → CF_Animation* (idle animation)          │
│ Key: "walk"     → CF_Animation* (walk animation)          │
│ Key: "attack"   → CF_Animation* (attack animation)        │
│ Key: "death"    → CF_Animation* (death animation)         │
└─────────────────────────────────────────────────────────────┘
```

## PNG API Functions Research

### 1. PNG Loading Functions

#### cf_png_cache_load
```cpp
CF_API CF_Result CF_CALL cf_png_cache_load(const char* png_path, CF_Png* png);
```

**Purpose**: Loads PNG image from disk into cache
**Parameters**:
- `png_path`: File path to PNG image
- `png`: Pointer to CF_Png structure to populate

**Returns**: CF_Result indicating success/failure
**Behavior**:
- Checks cache first, loads from disk if not cached
- Populates CF_Png structure with image data
- Manages memory automatically

#### cf_png_cache_load_from_memory
```cpp
CF_API CF_Result CF_CALL cf_png_cache_load_from_memory(
    const char* png_path,
    const void* memory,
    size_t size,
    CF_Png* png
);
```

**Purpose**: Loads PNG from memory buffer into cache
**Parameters**:
- `png_path`: Identifier for the PNG data
- `memory`: Pointer to PNG data in memory
- `size`: Size of PNG data in bytes
- `png`: Pointer to CF_Png structure to populate

**Use Cases**:
- Loading from embedded resources
- Network-loaded images
- Procedurally generated PNG data

### 2. Animation Creation Functions

#### cf_make_png_cache_animation
```cpp
CF_API const CF_Animation* CF_CALL cf_make_png_cache_animation(
    const char* name,
    const CF_Png* pngs,
    int pngs_count,
    const float* delays,
    int delays_count
);
```

**Purpose**: Creates animation from array of PNG frames
**Parameters**:
- `name`: Animation identifier
- `pngs`: Array of CF_Png structures
- `pngs_count`: Number of PNG frames
- `delays`: Array of frame delays (milliseconds)
- `delays_count`: Number of delay values

**Returns**: Pointer to created CF_Animation
**Behavior**:
- Creates animation with specified frames and timing
- Caches animation for reuse
- Returns existing animation if name already exists

#### cf_make_png_cache_animation_table
```cpp
CF_API const htbl CF_Animation** CF_CALL cf_make_png_cache_animation_table(
    const char* sprite_name,
    const CF_Animation* const* animations,
    int animations_count
);
```

**Purpose**: Creates animation table from multiple animations
**Parameters**:
- `sprite_name`: Identifier for the sprite
- `animations`: Array of CF_Animation pointers
- `animations_count`: Number of animations

**Returns**: Animation table hash table
**Behavior**:
- Creates lookup table for multiple animations
- Enables animation switching by name
- Caches table for reuse

### 3. Sprite Creation Functions

#### cf_make_png_cache_sprite
```cpp
CF_API CF_Sprite CF_CALL cf_make_png_cache_sprite(
    const char* sprite_name,
    const CF_Animation** table
);
```

**Purpose**: Creates sprite from animation table
**Parameters**:
- `sprite_name`: Identifier for the sprite
- `table`: Animation table (can be NULL for simple sprites)

**Returns**: CF_Sprite handle
**Behavior**:
- Creates sprite with animation support
- If table is NULL, uses sprite_name as PNG path
- Enables animation switching and frame management

## Implementation Architecture

### 1. Sprite Loading Pipeline

#### Phase 1: PNG Data Loading
```cpp
// Load PNG into cache
CF_Png png_data;
CF_Result result = cf_png_cache_load("assets/skeleton.png", &png_data);
if (result.code != CF_RESULT_OK) {
    // Handle error
    return false;
}

// Validate dimensions
if (png_data.w != 64 || png_data.h != 256) {
    // Handle dimension mismatch
    return false;
}
```

#### Phase 2: Animation Table Creation
```cpp
// Create individual animations for each direction
const CF_Animation* idle_anim = cf_make_png_cache_animation(
    "idle",
    &png_data,  // Single PNG for idle
    1,          // One frame
    &idle_delay, // Frame delay
    1           // One delay value
);

const CF_Animation* walk_anim = cf_make_png_cache_animation(
    "walk",
    &png_data,  // Single PNG for walk
    1,          // One frame
    &walk_delay, // Frame delay
    1           // One delay value
);

// Create animation table
const CF_Animation* animations[] = {idle_anim, walk_anim};
const char* animation_names[] = {"idle", "walk"};
const htbl CF_Animation** anim_table = cf_make_png_cache_animation_table(
    "skeleton_sprite",
    animations,
    2
);
```

#### Phase 3: Sprite Creation
```cpp
// Create sprite with animation support
CF_Sprite sprite = cf_make_png_cache_sprite("skeleton_sprite", anim_table);

// Set initial animation
cf_sprite_play(&sprite, "idle");
```

### 2. UV Coordinate System

#### Frame Calculation
```cpp
v2 calculateFrameUV(int frameIndex, Direction direction) {
    // For 64x256 sprite sheet with 4 directions
    float frameWidth = 64.0f / 64.0f;   // 1.0 (full width)
    float frameHeight = 64.0f / 256.0f;  // 0.25 (1/4 height)

    float u = 0.0f;  // Always start at left edge
    float v = (float)static_cast<int>(direction) * frameHeight;

    return v2(u, v);
}
```

#### UV Rectangle Definition
```cpp
struct UVRect {
    v2 min;  // Top-left UV coordinates
    v2 max;  // Bottom-right UV coordinates
};

UVRect getFrameUVRect(Direction direction) {
    v2 min = calculateFrameUV(0, direction);
    v2 max = min + v2(1.0f, 0.25f);  // Full width, 1/4 height
    return {min, max};
}
```

### 3. Memory Management Strategy

#### Cache Management
```cpp
class SpriteBatch {
private:
    CF_Png pngData;
    CF_Sprite sprite;
    const htbl CF_Animation** animationTable;

public:
    ~SpriteBatch() {
        // Framework handles most cleanup automatically
        // Only need to unload PNG if we loaded it manually
        if (pngData.id != ~0) {
            cf_png_cache_unload(pngData);
        }
    }
};
```

#### Resource Lifecycle
1. **Loading**: PNG loaded into cache, animation table created
2. **Usage**: Sprite references cached resources
3. **Cleanup**: Framework automatically manages sprite and animation memory
4. **Manual Cleanup**: Only PNG data needs manual unloading

## Performance Considerations

### 1. Caching Benefits
- **Automatic Memory Management**: Framework handles cache lifecycle
- **Reuse Optimization**: Multiple sprites can share same PNG data
- **Lazy Loading**: Resources loaded only when needed
- **Memory Efficiency**: Shared resources reduce memory footprint

### 2. Animation Table Benefits
- **Fast Lookup**: O(1) animation switching by name
- **Batch Operations**: Multiple animations managed together
- **Memory Sharing**: Animations share underlying PNG data
- **State Management**: Framework handles animation state

### 3. Sprite Management Benefits
- **Reference Counting**: Automatic cleanup when sprites are destroyed
- **Batch Rendering**: Framework can optimize multiple sprites
- **Transform Optimization**: Efficient matrix operations
- **GPU Memory**: Automatic texture management

## Error Handling Strategy

### 1. Loading Errors
```cpp
CF_Result result = cf_png_cache_load(path, &png);
if (result.code != CF_RESULT_OK) {
    printf("Failed to load PNG: %s\n", result.message);
    // Implement fallback or error recovery
    return false;
}
```

### 2. Validation Errors
```cpp
// Validate PNG dimensions
if (png.w != expected_width || png.h != expected_height) {
    printf("PNG dimensions mismatch: expected %dx%d, got %dx%d\n",
           expected_width, expected_height, png.w, png.h);
    return false;
}

// Validate pixel data
if (!png.pix) {
    printf("PNG pixel data is null\n");
    return false;
}
```

### 3. Animation Errors
```cpp
// Validate animation creation
const CF_Animation* anim = cf_make_png_cache_animation(name, pngs, count, delays, delays_count);
if (!anim) {
    printf("Failed to create animation: %s\n", name);
    return false;
}
```

## Testing Strategy

### 1. Unit Tests
- **PNG Loading**: Test various PNG formats and sizes
- **Animation Creation**: Test single and multiple frame animations
- **UV Calculations**: Test frame coordinate calculations
- **Error Handling**: Test invalid inputs and error conditions

### 2. Integration Tests
- **Sprite Creation**: Test complete sprite creation pipeline
- **Animation Switching**: Test animation state changes
- **Memory Management**: Test resource cleanup and reuse
- **Performance**: Test loading and rendering performance

### 3. Visual Tests
- **Frame Rendering**: Verify correct frame display
- **Direction Changes**: Test directional sprite rendering
- **Animation Playback**: Test animation timing and looping
- **Transform Operations**: Test position, scale, and rotation

## Next Steps

1. **Implementation Planning**: Break down into manageable tasks
2. **Prototype Development**: Create minimal working example
3. **Testing Framework**: Set up comprehensive testing
4. **Performance Profiling**: Measure and optimize performance
5. **Documentation**: Update implementation documentation
