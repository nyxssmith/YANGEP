#ifndef SPRITE_ANIMATION_LOADER_H
#define SPRITE_ANIMATION_LOADER_H

#include <cute.h>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <mutex>

using namespace Cute;

// Direction enum for directional sprites
enum class Direction
{
    UP = 0,
    LEFT = 1,
    DOWN = 2,
    RIGHT = 3
};

// Animation layout structure that defines how frames are arranged in a PNG
struct AnimationLayout
{
    std::string name;
    std::vector<std::string> filenames; // Filenames of the PNGs (layers, rendered in order)
    int frame_width;                    // Width of each frame in pixels
    int frame_height;                   // Height of each frame in pixels
    int frames_per_row;                 // How many frames in a horizontal row
    int frames_per_col;                 // How many frames in a vertical column
    std::vector<Direction> directions;  // Which directions this animation supports

    // Constructor with validation (with multiple filenames for layers)
    AnimationLayout(const std::string &n, const std::vector<std::string> &fns, int fw, int fh, int fpr, int fpc,
                    const std::vector<Direction> &dirs)
        : name(n), filenames(fns), frame_width(fw), frame_height(fh),
          frames_per_row(fpr), frames_per_col(fpc), directions(dirs)
    {
        // Validate layout
        if (frame_width <= 0 || frame_height <= 0 ||
            frames_per_row <= 0 || frames_per_col <= 0 ||
            directions.empty() || filenames.empty())
        {
            printf("Warning: Invalid animation layout for %s\n", name.c_str());
        }
    }

    // Constructor with validation (with single filename)
    AnimationLayout(const std::string &n, const std::string &fn, int fw, int fh, int fpr, int fpc,
                    const std::vector<Direction> &dirs)
        : name(n), filenames({fn}), frame_width(fw), frame_height(fh),
          frames_per_row(fpr), frames_per_col(fpc), directions(dirs)
    {
        // Validate layout
        if (frame_width <= 0 || frame_height <= 0 ||
            frames_per_row <= 0 || frames_per_col <= 0 ||
            directions.empty())
        {
            printf("Warning: Invalid animation layout for %s\n", name.c_str());
        }
    }

    // Constructor with validation (without filename, defaults to name + ".png")
    AnimationLayout(const std::string &n, int fw, int fh, int fpr, int fpc,
                    const std::vector<Direction> &dirs)
        : name(n), filenames({n + ".png"}), frame_width(fw), frame_height(fh),
          frames_per_row(fpr), frames_per_col(fpc), directions(dirs)
    {
        // Validate layout
        if (frame_width <= 0 || frame_height <= 0 ||
            frames_per_row <= 0 || frames_per_col <= 0 ||
            directions.empty())
        {
            printf("Warning: Invalid animation layout for %s\n", name.c_str());
        }
    }
};

// Animation frame structure
struct AnimationFrame
{
    std::vector<CF_Sprite> spriteLayers; // Sprite layers (rendered in order, bottom to top)
    CF_Sprite sprite;                    // Legacy: first sprite layer for backwards compatibility
    int frameIndex;                      // Frame number within animation
    Direction direction;                 // Direction this frame represents
    float delay;                         // Frame duration in milliseconds
    v2 offset;                           // Position offset within frame

    AnimationFrame() : frameIndex(0), direction(Direction::DOWN), delay(100.0f), offset(v2(0, 0)) {}

    // Get number of layers
    size_t getLayerCount() const { return spriteLayers.size(); }

    // Get sprite at layer index
    const CF_Sprite *getLayer(size_t index) const
    {
        if (index < spriteLayers.size())
            return &spriteLayers[index];
        return nullptr;
    }
};

// Animation structure
struct Animation
{
    std::string name;
    std::vector<AnimationFrame> frames;
    bool looping;
    float totalDuration;

    Animation() : looping(true), totalDuration(0.0f) {}

    // Get frame by index and direction
    const AnimationFrame *getFrame(int frameIndex, Direction direction) const;

    // Get frame by index only (for non-directional animations)
    const AnimationFrame *getFrame(int frameIndex) const;

    // Calculate total duration
    void calculateDuration();
};

// Animation table structure
struct AnimationTable
{
    std::map<std::string, Animation> animations;

    // Get animation by name
    const Animation *getAnimation(const std::string &name) const;

    // Add new animation
    void addAnimation(const std::string &name, const Animation &animation);

    // Check if animation exists
    bool hasAnimation(const std::string &name) const;

    // Get all animation names
    std::vector<std::string> getAnimationNames() const;
};

// Main sprite animation loader class that extends the existing PNG system
class SpriteAnimationLoader
{
private:
    // Static cache shared across all instances for PNG data
    static std::map<std::string, std::vector<uint8_t>> s_pngCache;

    // Static mutex for thread-safe cache access
    static std::mutex s_cacheMutex;

    // Load PNG file and cache it (thread-safe)
    bool loadAndCachePNG(const std::string &png_path);

    // Get cached PNG data (thread-safe)
    const std::vector<uint8_t> *getCachedPNG(const std::string &png_path) const;

public:
    SpriteAnimationLoader();
    ~SpriteAnimationLoader();

    // Preload multiple PNG files in parallel using the job system
    // This loads files into cache but doesn't create sprites (which must be done on main thread)
    void preloadPNGsParallel(const std::vector<std::string> &png_paths);

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

    // Create complete animation from layout with multiple layers
    Animation createAnimationWithLayers(const std::string &name,
                                        const std::string &base_path,
                                        const AnimationLayout &layout,
                                        float frameDelay = 100.0f);

    // Load multiple animations and create animation table
    AnimationTable loadAnimationTable(const std::string &base_path,
                                      const std::vector<AnimationLayout> &layouts);

    // Clear PNG cache (static - affects all instances)
    static void clearCache();

    // Get cache statistics (static - global cache info)
    static size_t getCacheSize();
    static size_t getCachedPNGCount();

    // Check if PNG is cached (static)
    static bool isPNGCached(const std::string &png_path);

    // Preload PNG files into cache in parallel (static - can be called without instance)
    static void preloadPNGsIntoCache(const std::vector<std::string> &png_paths);
};

// Predefined animation layouts for common sprite sheet formats
namespace AnimationLayouts
{
    // Idle animation: 4 directions, 1 frame each (256x64 PNG)
    extern const AnimationLayout IDLE_4_DIRECTIONS;

    // Walkcycle animation: 4 directions, 9 frames each (576x64 PNG)
    extern const AnimationLayout WALKCYCLE_4_DIRECTIONS_9_FRAMES;

}

#endif // SPRITE_ANIMATION_LOADER_H
