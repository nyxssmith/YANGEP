#ifndef SPRITE_SHEET_SPLITTER_H
#define SPRITE_SHEET_SPLITTER_H

#include <cute.h>
#include <cute_png_cache.h>
#include <vector>

using namespace Cute;

// Direction enum for directional sprites
enum class Direction {
    UP = 0,
    LEFT = 1,
    DOWN = 2,
    RIGHT = 3
};

// Virtual PNG structure that points to a region of the original sprite sheet
struct VirtualPNG {
    CF_Png png;           // The virtual PNG structure
    int frameIndex;       // Frame index within the direction
    Direction direction;   // Which direction this frame represents
    bool isValid;         // Whether this virtual PNG is valid
};

// Sprite sheet splitter that creates virtual PNGs from sprite sheet regions
class SpriteSheetSplitter {
public:
    SpriteSheetSplitter();
    ~SpriteSheetSplitter();

    // Load a sprite sheet and prepare for frame extraction
    bool loadSpriteSheet(const char* path, int frameWidth, int frameHeight, int framesPerDirection);

    // Get a specific frame as a virtual PNG
    VirtualPNG getFramePNG(Direction direction, int frameIndex = 0);

    // Get all frames for a specific direction
    std::vector<VirtualPNG> getAllFramesForDirection(Direction direction);

    // Check if the splitter is ready
    bool isReady() const;

    // Get frame dimensions
    int getFrameWidth() const;
    int getFrameHeight() const;
    int getFramesPerDirection() const;

    // Unload the sprite sheet
    void unloadSpriteSheet();

private:
    // Original sprite sheet data
    CF_Png originalPng;
    bool loaded;

    // Frame configuration
    int frameWidth;
    int frameHeight;
    int framesPerDirection;

    // Virtual PNGs for each frame
    std::vector<VirtualPNG> virtualPngs;

    // Private methods
    void createVirtualPNGs();
    VirtualPNG createVirtualPNG(Direction direction, int frameIndex);
    void cleanup();
};

#endif // SPRITE_SHEET_SPLITTER_H
