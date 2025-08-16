#include "SpriteSheetSplitter.h"
#include <cute.h>
#include <cute_png_cache.h>
#include <cstdlib>

using namespace Cute;

// Constructor
SpriteSheetSplitter::SpriteSheetSplitter()
    : loaded(false), frameWidth(0), frameHeight(0), framesPerDirection(0)
{
    // Initialize original PNG
    originalPng = cf_png_defaults();
}

// Destructor
SpriteSheetSplitter::~SpriteSheetSplitter() {
    cleanup();
}

// Load a sprite sheet and prepare for frame extraction
bool SpriteSheetSplitter::loadSpriteSheet(const char* path, int fw, int fh, int framesPerDir) {
    if (!path) {
        printf("FATAL ERROR: Invalid sprite sheet path\n");
        exit(1);
    }

    // Clean up any existing data
    cleanup();

    // Store frame configuration
    frameWidth = fw;
    frameHeight = fh;
    framesPerDirection = framesPerDir;

    // Load the original sprite sheet PNG
    CF_Result result = cf_png_cache_load(path, &originalPng);
    if (cf_is_error(result)) {
        printf("FATAL ERROR: Failed to load sprite sheet: %s - %s\n", path,
               result.details ? result.details : "Unknown error");
        exit(1);
    }

    // Validate dimensions
    int expectedWidth = frameWidth * framesPerDirection;
    int expectedHeight = frameHeight * 4; // 4 directions (UP, LEFT, DOWN, RIGHT)

    if (originalPng.w != expectedWidth || originalPng.h != expectedHeight) {
        printf("FATAL ERROR: Sprite sheet dimensions mismatch: expected %dx%d, got %dx%d\n",
               expectedWidth, expectedHeight, originalPng.w, originalPng.h);
        printf("Frame config: %dx%d, %d frames per direction\n", frameWidth, frameHeight, framesPerDirection);
        cf_png_cache_unload(originalPng);
        originalPng = cf_png_defaults();
        exit(1);
    }

    printf("Successfully loaded sprite sheet: %s (%dx%d)\n", path, originalPng.w, originalPng.h);
    printf("Frame config: %dx%d, %d frames per direction\n", frameWidth, frameHeight, framesPerDirection);

    loaded = true;

    // Create virtual PNGs for all frames
    createVirtualPNGs();

    return true;
}

// Create virtual PNGs for all frames
void SpriteSheetSplitter::createVirtualPNGs() {
    if (!loaded) return;

    // Calculate total number of frames
    int totalFrames = 4 * framesPerDirection; // 4 directions × frames per direction

    // Resize virtual PNGs vector
    virtualPngs.resize(totalFrames);

    // Create virtual PNGs for each direction and frame
    for (int direction = 0; direction < 4; direction++) {
        for (int frame = 0; frame < framesPerDirection; frame++) {
            int index = direction * framesPerDirection + frame;
            virtualPngs[index] = createVirtualPNG(static_cast<Direction>(direction), frame);
        }
    }

    printf("Created %d virtual PNGs\n", totalFrames);
}

// Create a virtual PNG for a specific direction and frame
VirtualPNG SpriteSheetSplitter::createVirtualPNG(Direction direction, int frameIndex) {
    VirtualPNG virtualPng;
    virtualPng.frameIndex = frameIndex;
    virtualPng.direction = direction;
    virtualPng.isValid = false;

    if (!loaded) return virtualPng;

    // Calculate pixel offsets within the original sprite sheet
    int startX = frameIndex * frameWidth;
    int startY = static_cast<int>(direction) * frameHeight;

    // Validate bounds
    if (startX + frameWidth > originalPng.w || startY + frameHeight > originalPng.h) {
        printf("ERROR: Frame %d for direction %d is out of bounds\n", frameIndex, static_cast<int>(direction));
        return virtualPng;
    }

    // Create virtual PNG structure
    virtualPng.png = cf_png_defaults();
    virtualPng.png.path = originalPng.path;  // Share the same path
    virtualPng.png.id = originalPng.id;      // Share the same ID
    virtualPng.png.w = frameWidth;           // Frame width
    virtualPng.png.h = frameHeight;          // Frame height

    // Calculate pixel data offset
    // Note: This is a simplified approach - we're pointing to the start of the frame region
    // In a real implementation, we might need to handle pixel format conversion
    virtualPng.png.pix = originalPng.pix + (startY * originalPng.w + startX);

    virtualPng.isValid = true;

    printf("Created virtual PNG: direction %d, frame %d, size %dx%d\n",
           static_cast<int>(direction), frameIndex, frameWidth, frameHeight);

    return virtualPng;
}

// Get a specific frame as a virtual PNG
VirtualPNG SpriteSheetSplitter::getFramePNG(Direction direction, int frameIndex) {
    if (!loaded || frameIndex < 0 || frameIndex >= framesPerDirection) {
        return VirtualPNG{cf_png_defaults(), 0, direction, false};
    }

    int index = static_cast<int>(direction) * framesPerDirection + frameIndex;
    if (index >= 0 && index < static_cast<int>(virtualPngs.size())) {
        return virtualPngs[index];
    }

    return VirtualPNG{cf_png_defaults(), 0, direction, false};
}

// Get all frames for a specific direction
std::vector<VirtualPNG> SpriteSheetSplitter::getAllFramesForDirection(Direction direction) {
    std::vector<VirtualPNG> frames;

    if (!loaded) return frames;

    int startIndex = static_cast<int>(direction) * framesPerDirection;
    for (int i = 0; i < framesPerDirection; i++) {
        int index = startIndex + i;
        if (index >= 0 && index < static_cast<int>(virtualPngs.size())) {
            frames.push_back(virtualPngs[index]);
        }
    }

    return frames;
}

// Check if the splitter is ready
bool SpriteSheetSplitter::isReady() const {
    return loaded && !virtualPngs.empty();
}

// Get frame dimensions
int SpriteSheetSplitter::getFrameWidth() const {
    return frameWidth;
}

int SpriteSheetSplitter::getFrameHeight() const {
    return frameHeight;
}

int SpriteSheetSplitter::getFramesPerDirection() const {
    return framesPerDirection;
}

// Cleanup resources
void SpriteSheetSplitter::cleanup() {
    if (loaded && originalPng.id != ~0) {
        cf_png_cache_unload(originalPng);
        originalPng = cf_png_defaults();
        loaded = false;
    }

    virtualPngs.clear();
    frameWidth = 0;
    frameHeight = 0;
    framesPerDirection = 0;
}

// Unload the sprite sheet
void SpriteSheetSplitter::unloadSpriteSheet() {
    cleanup();
}
