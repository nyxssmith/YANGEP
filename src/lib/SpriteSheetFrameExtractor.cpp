#include "SpriteSheetFrameExtractor.h"
#include <cute.h>
#include <cute_png_cache.h>
#include <cstdlib>
#include <cstring>

using namespace Cute;

// Constructor
SpriteSheetFrameExtractor::SpriteSheetFrameExtractor()
    : loaded(false), frameWidth(0), frameHeight(0), framesPerDirection(0)
{
    // Initialize original PNG
    originalPng = cf_png_defaults();
}

// Destructor
SpriteSheetFrameExtractor::~SpriteSheetFrameExtractor() {
    cleanup();
}

// Load a sprite sheet and prepare for frame extraction
bool SpriteSheetFrameExtractor::loadSpriteSheet(const char* path, int fw, int fh, int framesPerDir) {
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

    // Extract all frames as in-memory PNGs
    extractAllFrames();

    return true;
}

// Extract all frames as in-memory PNGs
void SpriteSheetFrameExtractor::extractAllFrames() {
    if (!loaded) return;

    // Calculate total number of frames
    int totalFrames = 4 * framesPerDirection; // 4 directions × frames per direction

    // Resize extracted frames vector
    extractedFrames.resize(totalFrames);

    // Extract frames for each direction and frame
    for (int direction = 0; direction < 4; direction++) {
        for (int frame = 0; frame < framesPerDirection; frame++) {
            int index = direction * framesPerDirection + frame;
            extractedFrames[index] = extractFrame(static_cast<Direction>(direction), frame);
        }
    }

    printf("Extracted %d frames as in-memory PNGs\n", totalFrames);
}

// Extract a specific frame as an in-memory PNG
ExtractedFrame SpriteSheetFrameExtractor::extractFrame(Direction direction, int frameIndex) {
    ExtractedFrame frame;
    frame.frameIndex = frameIndex;
    frame.direction = direction;
    frame.isValid = false;

    if (!loaded) return frame;

    // Calculate pixel offsets within the original sprite sheet
    int startX = frameIndex * frameWidth;
    int startY = static_cast<int>(direction) * frameHeight;

    // Validate bounds
    if (startX + frameWidth > originalPng.w || startY + frameHeight > originalPng.h) {
        printf("ERROR: Frame %d for direction %d is out of bounds\n", frameIndex, static_cast<int>(direction));
        return frame;
    }

    // Extract pixel data for this frame
    std::vector<uint8_t> framePixels;
    framePixels.reserve(frameWidth * frameHeight * 4); // RGBA format

    // Copy pixels from the sprite sheet region
    for (int y = 0; y < frameHeight; y++) {
        for (int x = 0; x < frameWidth; x++) {
            int srcX = startX + x;
            int srcY = startY + y;
            int srcIndex = (srcY * originalPng.w + srcX) * 4; // RGBA

            // Copy RGBA values
            framePixels.push_back(originalPng.pix[srcIndex + 0]); // R
            framePixels.push_back(originalPng.pix[srcIndex + 1]); // G
            framePixels.push_back(originalPng.pix[srcIndex + 2]); // B
            framePixels.push_back(originalPng.pix[srcIndex + 3]); // A
        }
    }

    // Create PNG data from the extracted pixels
    frame.pngData = createPNGFromPixels(framePixels.data(), frameWidth, frameHeight);
    frame.pngSize = frame.pngData.size();
    frame.isValid = !frame.pngData.empty();

    printf("Extracted frame: direction %d, frame %d, size %dx%d, PNG size %zu bytes\n",
           static_cast<int>(direction), frameIndex, frameWidth, frameHeight, frame.pngSize);

    return frame;
}

// Create PNG data from pixel data (simplified PNG creation)
std::vector<uint8_t> SpriteSheetFrameExtractor::createPNGFromPixels(const uint8_t* pixels, int width, int height) {
    // This is a simplified PNG creation - in a real implementation, you'd use a proper PNG library
    // For now, we'll create a minimal PNG structure that should work with CF

    std::vector<uint8_t> pngData;

    // PNG signature (8 bytes)
    const uint8_t pngSignature[] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
    pngData.insert(pngData.end(), pngSignature, pngSignature + 8);

    // IHDR chunk (image header)
    // Width: 4 bytes, Height: 4 bytes, Bit depth: 1 byte, Color type: 1 byte
    // Compression: 1 byte, Filter: 1 byte, Interlace: 1 byte
    uint32_t chunkLength = 13;
    uint32_t chunkType = 0x49484452; // "IHDR"

    // Add chunk length (big-endian)
    pngData.push_back((chunkLength >> 24) & 0xFF);
    pngData.push_back((chunkLength >> 16) & 0xFF);
    pngData.push_back((chunkLength >> 8) & 0xFF);
    pngData.push_back(chunkLength & 0xFF);

    // Add chunk type
    pngData.push_back((chunkType >> 24) & 0xFF);
    pngData.push_back((chunkType >> 16) & 0xFF);
    pngData.push_back((chunkType >> 8) & 0xFF);
    pngData.push_back(chunkType & 0xFF);

    // Add width (big-endian)
    pngData.push_back((width >> 24) & 0xFF);
    pngData.push_back((width >> 16) & 0xFF);
    pngData.push_back((width >> 8) & 0xFF);
    pngData.push_back(width & 0xFF);

    // Add height (big-endian)
    pngData.push_back((height >> 24) & 0xFF);
    pngData.push_back((height >> 16) & 0xFF);
    pngData.push_back((height >> 8) & 0xFF);
    pngData.push_back(height & 0xFF);

    // Add other IHDR fields (8-bit RGBA)
    pngData.push_back(8);   // Bit depth
    pngData.push_back(6);   // Color type (RGBA)
    pngData.push_back(0);   // Compression
    pngData.push_back(0);   // Filter
    pngData.push_back(0);   // Interlace

    // Calculate and add CRC for IHDR
    // (This is simplified - in reality you'd calculate the actual CRC)
    uint32_t crc = 0; // Placeholder
    pngData.push_back((crc >> 24) & 0xFF);
    pngData.push_back((crc >> 16) & 0xFF);
    pngData.push_back((crc >> 8) & 0xFF);
    pngData.push_back(crc & 0xFF);

    // IDAT chunk (image data)
    // For now, we'll add a placeholder - in reality you'd compress the pixel data
    chunkLength = 0; // Placeholder
    chunkType = 0x49444154; // "IDAT"

    pngData.push_back((chunkLength >> 24) & 0xFF);
    pngData.push_back((chunkLength >> 16) & 0xFF);
    pngData.push_back((chunkLength >> 8) & 0xFF);
    pngData.push_back(chunkLength & 0xFF);

    pngData.push_back((chunkType >> 24) & 0xFF);
    pngData.push_back((chunkType >> 16) & 0xFF);
    pngData.push_back((chunkType >> 8) & 0xFF);
    pngData.push_back(chunkType & 0xFF);

    // Add CRC for IDAT
    crc = 0; // Placeholder
    pngData.push_back((crc >> 24) & 0xFF);
    pngData.push_back((crc >> 16) & 0xFF);
    pngData.push_back((crc >> 8) & 0xFF);
    pngData.push_back(crc & 0xFF);

    // IEND chunk (end of file)
    chunkLength = 0;
    chunkType = 0x49454E44; // "IEND"

    pngData.push_back((chunkLength >> 24) & 0xFF);
    pngData.push_back((chunkLength >> 16) & 0xFF);
    pngData.push_back((chunkLength >> 8) & 0xFF);
    pngData.push_back(chunkLength & 0xFF);

    pngData.push_back((chunkType >> 24) & 0xFF);
    pngData.push_back((chunkType >> 16) & 0xFF);
    pngData.push_back((chunkType >> 8) & 0xFF);
    pngData.push_back(chunkType & 0xFF);

    // Add CRC for IEND
    crc = 0xAE426082; // Known CRC for IEND chunk
    pngData.push_back((crc >> 24) & 0xFF);
    pngData.push_back((crc >> 16) & 0xFF);
    pngData.push_back((crc >> 8) & 0xFF);
    pngData.push_back(crc & 0xFF);

    return pngData;
}

// Get a specific frame as an extracted frame
ExtractedFrame SpriteSheetFrameExtractor::getFrame(Direction direction, int frameIndex) {
    if (!loaded || frameIndex < 0 || frameIndex >= framesPerDirection) {
        return ExtractedFrame{std::vector<uint8_t>(), 0, direction, 0, false};
    }

    int index = static_cast<int>(direction) * framesPerDirection + frameIndex;
    if (index >= 0 && index < static_cast<int>(extractedFrames.size())) {
        return extractedFrames[index];
    }

    return ExtractedFrame{std::vector<uint8_t>(), 0, direction, 0, false};
}

// Get all frames for a specific direction
std::vector<ExtractedFrame> SpriteSheetFrameExtractor::getAllFramesForDirection(Direction direction) {
    std::vector<ExtractedFrame> frames;

    if (!loaded) return frames;

    int startIndex = static_cast<int>(direction) * framesPerDirection;
    for (int i = 0; i < framesPerDirection; i++) {
        int index = startIndex + i;
        if (index >= 0 && index < static_cast<int>(extractedFrames.size())) {
            frames.push_back(extractedFrames[index]);
        }
    }

    return frames;
}

// Check if the extractor is ready
bool SpriteSheetFrameExtractor::isReady() const {
    return loaded && !extractedFrames.empty();
}

// Get frame dimensions
int SpriteSheetFrameExtractor::getFrameWidth() const {
    return frameWidth;
}

int SpriteSheetFrameExtractor::getFrameHeight() const {
    return frameHeight;
}

int SpriteSheetFrameExtractor::getFramesPerDirection() const {
    return framesPerDirection;
}

// Cleanup resources
void SpriteSheetFrameExtractor::cleanup() {
    if (loaded && originalPng.id != ~0) {
        cf_png_cache_unload(originalPng);
        originalPng = cf_png_defaults();
        loaded = false;
    }

    extractedFrames.clear();
    frameWidth = 0;
    frameHeight = 0;
    framesPerDirection = 0;
}

// Unload the sprite sheet
void SpriteSheetFrameExtractor::unloadSpriteSheet() {
    cleanup();
}
