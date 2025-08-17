#ifndef SPRITE_ANIMATION_LOADER_H
#define SPRITE_ANIMATION_LOADER_H

#include <cute.h>
#include <string>
#include <vector>
#include <map>
#include <memory>

using namespace Cute;

// Direction enum for directional sprites
enum class Direction {
    UP = 0,
    LEFT = 1,
    DOWN = 2,
    RIGHT = 3
};

// Animation layout structure that defines how frames are arranged in a PNG
struct AnimationLayout {
    std::string name;
    int frame_width;      // Width of each frame in pixels
    int frame_height;     // Height of each frame in pixels
    int frames_per_row;   // How many frames in a horizontal row
    int frames_per_col;   // How many frames in a vertical column
    std::vector<Direction> directions; // Which directions this animation supports

    // Constructor with validation
    AnimationLayout(const std::string& n, int fw, int fh, int fpr, int fpc,
                   const std::vector<Direction>& dirs)
        : name(n), frame_width(fw), frame_height(fh),
          frames_per_row(fpr), frames_per_col(fpc), directions(dirs) {
        // Validate layout
        if (frame_width <= 0 || frame_height <= 0 ||
            frames_per_row <= 0 || frames_per_col <= 0 ||
            directions.empty()) {
            printf("Warning: Invalid animation layout for %s\n", name.c_str());
        }
    }
};

// Animation frame structure
struct AnimationFrame {
    CF_Sprite sprite;           // The actual sprite (from existing PNG system)
    int frameIndex;             // Frame number within animation
    Direction direction;         // Direction this frame represents
    float delay;                // Frame duration in milliseconds
    v2 offset;                  // Position offset within frame

    AnimationFrame() : frameIndex(0), direction(Direction::DOWN), delay(100.0f), offset(v2(0, 0)) {}
};

// Animation structure
struct Animation {
    std::string name;
    std::vector<AnimationFrame> frames;
    bool looping;
    float totalDuration;

    Animation() : looping(true), totalDuration(0.0f) {}

    // Get frame by index and direction
    const AnimationFrame* getFrame(int frameIndex, Direction direction) const;

    // Get frame by index only (for non-directional animations)
    const AnimationFrame* getFrame(int frameIndex) const;

    // Calculate total duration
    void calculateDuration();
};

// Animation table structure
struct AnimationTable {
    std::map<std::string, Animation> animations;

    // Get animation by name
    const Animation* getAnimation(const std::string &name) const;

    // Add new animation
    void addAnimation(const std::string &name, const Animation &animation);

    // Check if animation exists
    bool hasAnimation(const std::string &name) const;

    // Get all animation names
    std::vector<std::string> getAnimationNames() const;
};

// Main sprite animation loader class that extends the existing PNG system
class SpriteAnimationLoader {
private:
    // Cache loaded PNG data to avoid reloading
    std::map<std::string, std::vector<uint8_t>> pngCache;

    // Load PNG file and cache it
    bool loadAndCachePNG(const std::string &png_path);

    // Get cached PNG data
    const std::vector<uint8_t>* getCachedPNG(const std::string &png_path) const;

public:
    SpriteAnimationLoader();
    ~SpriteAnimationLoader();

    // Extract a single sprite frame from PNG (useful for testing and advanced usage)
    CF_Sprite extractSpriteFrame(const std::string &png_path,
                                int frame_x, int frame_y,
                                int frame_width, int frame_height);

    // Load animation frames from sprite sheet using existing PNG system
    std::vector<CF_Sprite> loadAnimationFrames(const std::string &png_path,
                                              const AnimationLayout &layout);

    // Create complete animation from layout
    Animation createAnimation(const std::string &name,
                            const std::string &png_path,
                            const AnimationLayout &layout,
                            float frameDelay = 100.0f);

    // Load multiple animations and create animation table
    AnimationTable loadAnimationTable(const std::string &base_path,
                                    const std::vector<AnimationLayout> &layouts);

    // Clear PNG cache
    void clearCache();

    // Get cache statistics
    size_t getCacheSize() const;
    size_t getCachedPNGCount() const;

    // Check if PNG is cached
    bool isPNGCached(const std::string &png_path) const;
};

// Predefined animation layouts for common sprite sheet formats
namespace AnimationLayouts {
        // Idle animation: 4 directions, 1 frame each (256x64 PNG)
    extern const AnimationLayout IDLE_4_DIRECTIONS;

    // Walkcycle animation: 4 directions, 9 frames each (576x64 PNG)
    extern const AnimationLayout WALKCYCLE_4_DIRECTIONS_9_FRAMES;


}

#endif // SPRITE_ANIMATION_LOADER_H
