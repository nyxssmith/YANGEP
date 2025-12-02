#include "SpriteAnimationLoader.h"
#include "DebugPrint.h"
#include <cute.h>
#include <spng.h>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <unistd.h>

using namespace Cute;

// Predefined animation layouts
namespace AnimationLayouts
{
    // Idle: 1 frame × 4 directions each = 4 total frames (64x256 PNG)
    const AnimationLayout IDLE_4_DIRECTIONS(
        "idle", 64, 64, 1, 4,
        {Direction::UP, Direction::LEFT, Direction::DOWN, Direction::RIGHT});

    // Walkcycle: 4 directions × 9 frames each = 36 total frames (576x256 PNG)
    const AnimationLayout WALKCYCLE_4_DIRECTIONS_9_FRAMES(
        "walkcycle", 64, 64, 9, 4,
        {Direction::UP, Direction::LEFT, Direction::DOWN, Direction::RIGHT});

}

// Constructor
SpriteAnimationLoader::SpriteAnimationLoader()
{
    DebugPrint::Print("SpriteAnimationLoader", "SpriteAnimationLoader: Initialized\n");
}

// Destructor
SpriteAnimationLoader::~SpriteAnimationLoader()
{
    clearCache();
}

// Clear PNG cache
void SpriteAnimationLoader::clearCache()
{
    DebugPrint::Print("SpriteAnimationLoader", "SpriteAnimationLoader: Clearing PNG cache (%zu PNGs)\n", pngCache.size());
    // std::vector manages its own memory, just clear the map
    pngCache.clear();
}

// Load PNG file and cache it
bool SpriteAnimationLoader::loadAndCachePNG(const std::string &png_path)
{
    if (isPNGCached(png_path))
    {
        DebugPrint::Print("SpriteAnimationLoader", "SpriteAnimationLoader: PNG already cached: %s\n", png_path.c_str());
        return true;
    }

    DebugPrint::Print("SpriteAnimationLoader", "SpriteAnimationLoader: Loading and caching PNG: %s\n", png_path.c_str());

    // Read the entire PNG file using Cute Framework's VFS (same as tsx.cpp)
    size_t file_size = 0;
    void *file_data = cf_fs_read_entire_file_to_memory(png_path.c_str(), &file_size);

    if (file_data == nullptr)
    {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != nullptr)
        {
            DebugPrint::Print("SpriteAnimationLoader", "SpriteAnimationLoader: Failed to read PNG file: %s (cwd: %s)\n", png_path.c_str(), cwd);
        }
        else
        {
            DebugPrint::Print("SpriteAnimationLoader", "SpriteAnimationLoader: Failed to read PNG file: %s (cwd: unknown)\n", png_path.c_str());
        }
        return false;
    }

    if (file_size == 0)
    {
        DebugPrint::Print("SpriteAnimationLoader", "SpriteAnimationLoader: PNG file is empty: %s\n", png_path.c_str());
        cf_free(file_data);
        return false;
    }

    // Copy the data to our cache
    std::vector<uint8_t> png_data(file_size);
    memcpy(png_data.data(), file_data, file_size);

    // Free the original file data
    cf_free(file_data);

    // Store in cache
    pngCache[png_path] = std::move(png_data);

    DebugPrint::Print("SpriteAnimationLoader", "SpriteAnimationLoader: Successfully cached PNG: %s (%zu bytes)\n",
                      png_path.c_str(), file_size);
    return true;
}

// Get cached PNG data
const std::vector<uint8_t> *SpriteAnimationLoader::getCachedPNG(const std::string &png_path) const
{
    auto it = pngCache.find(png_path);
    if (it != pngCache.end())
    {
        return &it->second;
    }
    return nullptr;
}

// Check if PNG is cached
bool SpriteAnimationLoader::isPNGCached(const std::string &png_path) const
{
    return pngCache.find(png_path) != pngCache.end();
}

// Get cache statistics
size_t SpriteAnimationLoader::getCacheSize() const
{
    size_t total_size = 0;
    for (const auto &entry : pngCache)
    {
        total_size += entry.second.size();
    }
    return total_size;
}

size_t SpriteAnimationLoader::getCachedPNGCount() const
{
    return pngCache.size();
}

// Extract sprite frame from PNG using the proven system from tsx.cpp
CF_Sprite SpriteAnimationLoader::extractSpriteFrame(const std::string &png_path,
                                                    int frame_x, int frame_y,
                                                    int frame_width, int frame_height)
{
    DebugPrint::Print("SpriteAnimationLoader", "SpriteAnimationLoader: Extracting frame (%d, %d) size (%dx%d) from %s\n",
                      frame_x, frame_y, frame_width, frame_height, png_path.c_str());

    // Ensure PNG is loaded and cached
    if (!loadAndCachePNG(png_path))
    {
        DebugPrint::Print("SpriteAnimationLoader", "SpriteAnimationLoader: Failed to load PNG for frame extraction: %s\n", png_path.c_str());
        return cf_sprite_defaults();
    }

    const std::vector<uint8_t> *png_data = getCachedPNG(png_path);
    if (!png_data || png_data->empty())
    {
        DebugPrint::Print("SpriteAnimationLoader", "SpriteAnimationLoader: No cached PNG data for: %s\n", png_path.c_str());
        return cf_sprite_defaults();
    }

    // Initialize libspng context (same as tsx.cpp)
    spng_ctx *ctx = spng_ctx_new(0);
    if (ctx == nullptr)
    {
        DebugPrint::Print("SpriteAnimationLoader", "SpriteAnimationLoader: Failed to create spng context\n");
        return cf_sprite_defaults();
    }

    // Set PNG data
    int ret = spng_set_png_buffer(ctx, png_data->data(), png_data->size());
    if (ret != 0)
    {
        DebugPrint::Print("SpriteAnimationLoader", "SpriteAnimationLoader: spng_set_png_buffer error: %s\n", spng_strerror(ret));
        spng_ctx_free(ctx);
        return cf_sprite_defaults();
    }

    // Get image header
    struct spng_ihdr ihdr;
    ret = spng_get_ihdr(ctx, &ihdr);
    if (ret != 0)
    {
        DebugPrint::Print("SpriteAnimationLoader", "SpriteAnimationLoader: spng_get_ihdr error: %s\n", spng_strerror(ret));
        spng_ctx_free(ctx);
        return cf_sprite_defaults();
    }

    DebugPrint::Print("SpriteAnimationLoader", "SpriteAnimationLoader: PNG dimensions: %dx%d, bit depth: %d, color type: %d\n",
                      ihdr.width, ihdr.height, ihdr.bit_depth, ihdr.color_type);

    // Validate frame bounds
    if (frame_x + frame_width > (int)ihdr.width || frame_y + frame_height > (int)ihdr.height)
    {
        DebugPrint::Print("SpriteAnimationLoader", "SpriteAnimationLoader: Frame bounds exceed image dimensions. Frame: (%d,%d)+(%dx%d), Image: %dx%d\n",
                          frame_x, frame_y, frame_width, frame_height, ihdr.width, ihdr.height);
        spng_ctx_free(ctx);
        return cf_sprite_defaults();
    }

    // Decode to RGBA8 (same as tsx.cpp)
    size_t image_size;
    ret = spng_decoded_image_size(ctx, SPNG_FMT_RGBA8, &image_size);
    if (ret != 0)
    {
        DebugPrint::Print("SpriteAnimationLoader", "SpriteAnimationLoader: spng_decoded_image_size error: %s\n", spng_strerror(ret));
        spng_ctx_free(ctx);
        return cf_sprite_defaults();
    }

    // Allocate buffer for the full image
    std::vector<uint8_t> full_image(image_size);
    ret = spng_decode_image(ctx, full_image.data(), image_size, SPNG_FMT_RGBA8, 0);
    if (ret != 0)
    {
        DebugPrint::Print("SpriteAnimationLoader", "SpriteAnimationLoader: spng_decode_image error: %s\n", spng_strerror(ret));
        spng_ctx_free(ctx);
        return cf_sprite_defaults();
    }

    // Clean up libspng resources
    spng_ctx_free(ctx);

    // Create buffer for cropped frame (RGBA format)
    std::vector<CF_Pixel> frame_pixels(frame_width * frame_height);

    // Copy the frame region from the full image (same logic as tsx.cpp)
    for (int y = 0; y < frame_height; y++)
    {
        for (int x = 0; x < frame_width; x++)
        {
            int src_x = frame_x + x;
            int src_y = frame_y + y;
            int src_index = (src_y * ihdr.width + src_x) * 4; // 4 bytes per pixel (RGBA)
            int dst_index = y * frame_width + x;

            // Copy RGBA data
            frame_pixels[dst_index].colors.r = full_image[src_index + 0];
            frame_pixels[dst_index].colors.g = full_image[src_index + 1];
            frame_pixels[dst_index].colors.b = full_image[src_index + 2];
            frame_pixels[dst_index].colors.a = full_image[src_index + 3];
        }
    }

    // Create sprite from pixel data using Cute Framework (same as tsx.cpp)
    CF_Sprite frame_sprite = cf_make_easy_sprite_from_pixels(frame_pixels.data(), frame_width, frame_height);

    DebugPrint::Print("SpriteAnimationLoader", "SpriteAnimationLoader: Successfully extracted frame (%d, %d) -> pixel region (%d, %d) size (%dx%d)\n",
                      frame_x, frame_y, frame_x, frame_y, frame_width, frame_height);

    return frame_sprite;
}

// Load animation frames from sprite sheet using existing PNG system
std::vector<CF_Sprite> SpriteAnimationLoader::loadAnimationFrames(const std::string &png_path,
                                                                  const AnimationLayout &layout)
{
    DebugPrint::Print("SpriteAnimationLoader", "SpriteAnimationLoader: Loading animation frames for %s from %s\n",
                      layout.name.c_str(), png_path.c_str());

    std::vector<CF_Sprite> frames;

    // Extract frames for each direction
    for (int dir = 0; dir < layout.directions.size(); dir++)
    {
        for (int frame = 0; frame < layout.frames_per_row; frame++)
        {
            // Calculate frame coordinates (same logic as tile coordinates in tsx.cpp)
            int frame_x = frame * layout.frame_width;
            int frame_y = dir * layout.frame_height;

            DebugPrint::Print("SpriteAnimationLoader", "SpriteAnimationLoader: Extracting frame %d, direction %d at (%d, %d)\n",
                              frame, static_cast<int>(layout.directions[dir]), frame_x, frame_y);

            // Use the existing working PNG extraction method
            CF_Sprite frame_sprite = extractSpriteFrame(png_path, frame_x, frame_y,
                                                        layout.frame_width, layout.frame_height);

            // Assume the sprite creation succeeded if we got here
            frames.push_back(frame_sprite);
            DebugPrint::Print("SpriteAnimationLoader", "SpriteAnimationLoader: Added frame %zu\n", frames.size());
        }
    }

    DebugPrint::Print("SpriteAnimationLoader", "SpriteAnimationLoader: Loaded %zu frames for animation %s\n", frames.size(), layout.name.c_str());
    return frames;
}

// Create complete animation from layout (supports multiple layers)
Animation SpriteAnimationLoader::createAnimation(const std::string &name,
                                                 const std::string &png_path,
                                                 const AnimationLayout &layout,
                                                 float frameDelay)
{
    DebugPrint::Print("SpriteAnimationLoader", "SpriteAnimationLoader: Creating animation %s from %s\n", name.c_str(), png_path.c_str());

    Animation anim;
    anim.name = name;
    anim.looping = true;

    // Load all frames using the existing PNG system
    std::vector<CF_Sprite> sprites = loadAnimationFrames(png_path, layout);

    if (sprites.empty())
    {
        DebugPrint::Print("SpriteAnimationLoader", "SpriteAnimationLoader: No frames loaded for animation %s\n", name.c_str());
        return anim;
    }

    // Convert sprites to animation frames (single layer)
    for (size_t i = 0; i < sprites.size(); i++)
    {
        AnimationFrame frame;
        frame.sprite = sprites[i];
        frame.spriteLayers.push_back(sprites[i]); // Add as first layer
        frame.frameIndex = i % layout.frames_per_row;
        frame.direction = layout.directions[i / layout.frames_per_row];
        frame.delay = frameDelay;
        frame.offset = v2(0, 0); // No offset for now

        anim.frames.push_back(frame);
    }

    // Calculate total duration
    anim.calculateDuration();

    DebugPrint::Print("SpriteAnimationLoader", "SpriteAnimationLoader: Created animation %s with %zu frames, duration: %.2fms\n",
                      name.c_str(), anim.frames.size(), anim.totalDuration);

    return anim;
}

// Create complete animation from layout with multiple layers
Animation SpriteAnimationLoader::createAnimationWithLayers(const std::string &name,
                                                           const std::string &base_path,
                                                           const AnimationLayout &layout,
                                                           float frameDelay)
{
    DebugPrint::Print("SpriteAnimationLoader", "SpriteAnimationLoader: Creating layered animation %s with %zu layers\n",
                      name.c_str(), layout.filenames.size());

    Animation anim;
    anim.name = name;
    anim.looping = true;

    // Load frames from all layers
    std::vector<std::vector<CF_Sprite>> allLayerSprites;

    for (const auto &filename : layout.filenames)
    {
        std::string png_path = base_path + "/" + filename;
        DebugPrint::Print("SpriteAnimationLoader", "SpriteAnimationLoader: Loading layer from %s\n", png_path.c_str());

        std::vector<CF_Sprite> layerSprites = loadAnimationFrames(png_path, layout);

        if (layerSprites.empty())
        {
            DebugPrint::Print("SpriteAnimationLoader", "SpriteAnimationLoader: WARNING: No frames loaded from layer %s\n", filename.c_str());
            continue;
        }

        allLayerSprites.push_back(layerSprites);
    }

    if (allLayerSprites.empty())
    {
        DebugPrint::Print("SpriteAnimationLoader", "SpriteAnimationLoader: ERROR: No layers loaded for animation %s\n", name.c_str());
        return anim;
    }

    // Use the first layer's frame count as reference
    size_t frameCount = allLayerSprites[0].size();

    // Create animation frames with all layers
    for (size_t i = 0; i < frameCount; i++)
    {
        AnimationFrame frame;
        frame.frameIndex = i % layout.frames_per_row;
        frame.direction = layout.directions[i / layout.frames_per_row];
        frame.delay = frameDelay;
        frame.offset = v2(0, 0);

        // Add sprites from each layer
        for (size_t layerIdx = 0; layerIdx < allLayerSprites.size(); layerIdx++)
        {
            if (i < allLayerSprites[layerIdx].size())
            {
                frame.spriteLayers.push_back(allLayerSprites[layerIdx][i]);
            }
        }

        // Set legacy sprite to first layer for backwards compatibility
        if (!frame.spriteLayers.empty())
        {
            frame.sprite = frame.spriteLayers[0];
        }

        anim.frames.push_back(frame);
    }

    // Calculate total duration
    anim.calculateDuration();

    DebugPrint::Print("SpriteAnimationLoader", "SpriteAnimationLoader: Created layered animation %s with %zu frames and %zu layers, duration: %.2fms\n",
                      name.c_str(), anim.frames.size(), allLayerSprites.size(), anim.totalDuration);

    return anim;
}

// Load multiple animations and create animation table
AnimationTable SpriteAnimationLoader::loadAnimationTable(const std::string &base_path,
                                                         const std::vector<AnimationLayout> &layouts)
{
    DebugPrint::Print("SpriteAnimationLoader", "SpriteAnimationLoader: Loading animation table from base path: %s\n", base_path.c_str());

    AnimationTable table;

    for (const auto &layout : layouts)
    {
        // Construct base PNG path for this animation
        // PhysFS requires absolute paths starting with /
        // Also, animations are in subdirectories (idle/, walkcycle/)
        std::string anim_base_path = "/" + base_path + "/" + layout.name;

        DebugPrint::Print("SpriteAnimationLoader", "SpriteAnimationLoader: Loading animation %s from %s with %zu layers\n",
                          layout.name.c_str(), anim_base_path.c_str(), layout.filenames.size());

        // Create animation with all layers
        Animation anim = createAnimationWithLayers(layout.name, anim_base_path, layout);

        if (!anim.frames.empty())
        {
            table.addAnimation(layout.name, anim);
            DebugPrint::Print("SpriteAnimationLoader", "SpriteAnimationLoader: Added animation %s to table\n", layout.name.c_str());
        }
        else
        {
            DebugPrint::Print("SpriteAnimationLoader", "SpriteAnimationLoader: Failed to create animation %s\n", layout.name.c_str());
        }
    }

    DebugPrint::Print("SpriteAnimationLoader", "SpriteAnimationLoader: Animation table loaded with %zu animations\n",
                      table.getAnimationNames().size());

    return table;
}

// Animation methods implementation
const AnimationFrame *Animation::getFrame(int frameIndex, Direction direction) const
{
    for (const auto &frame : frames)
    {
        if (frame.frameIndex == frameIndex && frame.direction == direction)
        {
            return &frame;
        }
    }
    return nullptr;
}

const AnimationFrame *Animation::getFrame(int frameIndex) const
{
    for (const auto &frame : frames)
    {
        if (frame.frameIndex == frameIndex)
        {
            return &frame;
        }
    }
    return nullptr;
}

void Animation::calculateDuration()
{
    totalDuration = 0.0f;
    for (const auto &frame : frames)
    {
        totalDuration += frame.delay;
    }
}

// AnimationTable methods implementation
const Animation *AnimationTable::getAnimation(const std::string &name) const
{
    auto it = animations.find(name);
    if (it != animations.end())
    {
        return &it->second;
    }
    return nullptr;
}

void AnimationTable::addAnimation(const std::string &name, const Animation &animation)
{
    animations[name] = animation;
}

bool AnimationTable::hasAnimation(const std::string &name) const
{
    return animations.find(name) != animations.end();
}

std::vector<std::string> AnimationTable::getAnimationNames() const
{
    std::vector<std::string> names;
    for (const auto &entry : animations)
    {
        names.push_back(entry.first);
    }
    return names;
}
