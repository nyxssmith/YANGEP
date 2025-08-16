#ifndef SPRITE_SHEET_FRAME_EXTRACTOR_H
#define SPRITE_SHEET_FRAME_EXTRACTOR_H

#include <cute.h>
#include <cute_png_cache.h>
#include <vector>
#include <string>

using namespace Cute;

// Direction enum for directional sprites
enum class Direction {
    UP = 0,
    LEFT = 1,
    DOWN = 2,
    RIGHT = 3
};

// Extracted frame data as in-memory PNG
struct ExtractedFrame {
    std::vector<uint8_t> pngData;  // Raw PNG data in memory
    size_t pngSize;                // Size of PNG data
    Direction direction;            // Which direction this frame represents
    int frameIndex;                // Frame index within the direction
    bool isValid;                  // Whether this frame is valid
};

// Sprite sheet frame extractor that creates in-memory PNGs
class SpriteSheetFrameExtractor {
public:
    SpriteSheetFrameExtractor();
    ~SpriteSheetFrameExtractor();

    // Load a sprite sheet and extract individual frames as PNGs
    bool loadSpriteSheet(const char* path, int frameWidth, int frameHeight, int framesPerDirection);

    // Get a specific frame as an in-memory PNG
    ExtractedFrame getFrame(Direction direction, int frameIndex = 0);

    // Get all frames for a specific direction
    std::vector<ExtractedFrame> getAllFramesForDirection(Direction direction);

    // Check if the extractor is ready
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

    // Extracted frames
    std::vector<ExtractedFrame> extractedFrames;

    // Private methods
    void extractAllFrames();
    ExtractedFrame extractFrame(Direction direction, int frameIndex);
    std::vector<uint8_t> createPNGFromPixels(const uint8_t* pixels, int width, int height);
    void cleanup();
};

#endif // SPRITE_SHEET_FRAME_EXTRACTOR_H
