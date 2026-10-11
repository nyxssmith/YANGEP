#include "SpriteAnimationLoader.h"
#include <cute.h>
#include <spng.h>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <unistd.h>
#include <cstdarg>

using namespace Cute;

// Mute verbose sprite loader logs unless enabled.
static bool g_sprite_loader_verbose = false;
static int s_printf(const char *fmt, ...)
{
    if (!g_sprite_loader_verbose)
        return 0;
    va_list args;
    va_start(args, fmt);
    int r = vprintf(fmt, args);
    va_end(args);
    fflush(stdout);
    return r;
}
#define printf s_printf

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
    printf("SpriteAnimationLoader: Initialized\n");
}

// Destructor
SpriteAnimationLoader::~SpriteAnimationLoader()
{
    clearCache();
}

// Clear PNG cache
void SpriteAnimationLoader::clearCache()
{
    printf("SpriteAnimationLoader: Clearing PNG cache (%zu PNGs)\n", pngCache.size());
    pngCache.clear();
    tableCache.clear();
}

// Read a PNG file, decode it to RGBA8 once, and cache it
bool SpriteAnimationLoader::loadAndCachePNG(const std::string &png_path)
{
    if (isPNGCached(png_path))
    {
        printf("SpriteAnimationLoader: PNG already cached: %s\n", png_path.c_str());
        return true;
    }

    printf("SpriteAnimationLoader: Loading and caching PNG: %s\n", png_path.c_str());

    // Read the entire PNG file using Cute Framework's VFS (same as tsx.cpp)
    size_t file_size = 0;
    void *file_data = cf_fs_read_entire_file_to_memory(png_path.c_str(), &file_size);

    if (file_data == nullptr)
    {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != nullptr)
        {
            printf("SpriteAnimationLoader: Failed to read PNG file: %s (cwd: %s)\n", png_path.c_str(), cwd);
        }
        else
        {
            printf("SpriteAnimationLoader: Failed to read PNG file: %s (cwd: unknown)\n", png_path.c_str());
        }
        return false;
    }

    if (file_size == 0)
    {
        printf("SpriteAnimationLoader: PNG file is empty: %s\n", png_path.c_str());
        cf_free(file_data);
        return false;
    }

    spng_ctx *ctx = spng_ctx_new(0);
    if (ctx == nullptr)
    {
        printf("SpriteAnimationLoader: Failed to create spng context\n");
        cf_free(file_data);
        return false;
    }

    DecodedPNG decoded;
    struct spng_ihdr ihdr;
    size_t image_size = 0;
    int ret = spng_set_png_buffer(ctx, file_data, file_size);
    if (ret == 0)
        ret = spng_get_ihdr(ctx, &ihdr);
    if (ret == 0)
        ret = spng_decoded_image_size(ctx, SPNG_FMT_RGBA8, &image_size);
    if (ret == 0)
    {
        decoded.rgba.resize(image_size);
        ret = spng_decode_image(ctx, decoded.rgba.data(), image_size, SPNG_FMT_RGBA8, 0);
    }

    spng_ctx_free(ctx);
    cf_free(file_data);

    if (ret != 0)
    {
        printf("SpriteAnimationLoader: Failed to decode PNG %s: %s\n", png_path.c_str(), spng_strerror(ret));
        return false;
    }

    decoded.width = (int)ihdr.width;
    decoded.height = (int)ihdr.height;
    pngCache[png_path] = std::move(decoded);

    printf("SpriteAnimationLoader: Successfully cached PNG: %s (%zu bytes)\n",
           png_path.c_str(), image_size);
    return true;
}

// Get cached decoded PNG
const SpriteAnimationLoader::DecodedPNG *SpriteAnimationLoader::getCachedPNG(const std::string &png_path) const
{
    auto it = pngCache.find(png_path);
    if (it != pngCache.end())
    {
        return &it->second;
    }
    return nullptr;
}

bool SpriteAnimationLoader::getPNGDimensions(const std::string &png_path, uint32_t &width, uint32_t &height)
{
    if (!loadAndCachePNG(png_path))
        return false;
    const DecodedPNG *png = getCachedPNG(png_path);
    if (!png)
        return false;
    width = (uint32_t)png->width;
    height = (uint32_t)png->height;
    return true;
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
        total_size += entry.second.rgba.size();
    }
    return total_size;
}

size_t SpriteAnimationLoader::getCachedPNGCount() const
{
    return pngCache.size();
}

// Extract sprite frame from the cached, already-decoded PNG
CF_Sprite SpriteAnimationLoader::extractSpriteFrame(const std::string &png_path,
                                                    int frame_x, int frame_y,
                                                    int frame_width, int frame_height)
{
    printf("SpriteAnimationLoader: Extracting frame (%d, %d) size (%dx%d) from %s\n",
           frame_x, frame_y, frame_width, frame_height, png_path.c_str());

    // Ensure PNG is loaded, decoded and cached
    if (!loadAndCachePNG(png_path))
    {
        printf("SpriteAnimationLoader: Failed to load PNG for frame extraction: %s\n", png_path.c_str());
        return cf_sprite_defaults();
    }

    const DecodedPNG *png = getCachedPNG(png_path);
    if (!png || png->rgba.empty())
    {
        printf("SpriteAnimationLoader: No cached PNG data for: %s\n", png_path.c_str());
        return cf_sprite_defaults();
    }

    // Validate frame bounds
    if (frame_x + frame_width > png->width || frame_y + frame_height > png->height)
    {
        printf("SpriteAnimationLoader: Frame bounds exceed image dimensions. Frame: (%d,%d)+(%dx%d), Image: %dx%d\n",
               frame_x, frame_y, frame_width, frame_height, png->width, png->height);
        return cf_sprite_defaults();
    }

    const std::vector<uint8_t> &full_image = png->rgba;

    // Create buffer for cropped frame (RGBA format)
    std::vector<CF_Pixel> frame_pixels(frame_width * frame_height);

    // Copy the frame region from the full image (same logic as tsx.cpp)
    for (int y = 0; y < frame_height; y++)
    {
        for (int x = 0; x < frame_width; x++)
        {
            int src_x = frame_x + x;
            int src_y = frame_y + y;
            int src_index = (src_y * png->width + src_x) * 4; // 4 bytes per pixel (RGBA)
            int dst_index = y * frame_width + x;

            // Copy RGBA data with premultiply-at-load (rgb *= a)
            uint8_t r = full_image[src_index + 0];
            uint8_t g = full_image[src_index + 1];
            uint8_t b = full_image[src_index + 2];
            uint8_t a = full_image[src_index + 3];
            // Round to nearest: (x*a + 127)/255
            frame_pixels[dst_index].colors.r = (uint8_t)((r * a + 127) / 255);
            frame_pixels[dst_index].colors.g = (uint8_t)((g * a + 127) / 255);
            frame_pixels[dst_index].colors.b = (uint8_t)((b * a + 127) / 255);
            frame_pixels[dst_index].colors.a = a;
        }
    }

    // Create sprite from pixel data using Cute Framework (same as tsx.cpp)
    CF_Sprite frame_sprite = cf_make_easy_sprite_from_pixels(frame_pixels.data(), frame_width, frame_height);

    printf("SpriteAnimationLoader: Successfully extracted frame (%d, %d) -> pixel region (%d, %d) size (%dx%d)\n",
           frame_x, frame_y, frame_x, frame_y, frame_width, frame_height);

    return frame_sprite;
}

// Load animation frames from sprite sheet using existing PNG system
std::vector<CF_Sprite> SpriteAnimationLoader::loadAnimationFrames(const std::string &png_path,
                                                                  const AnimationLayout &layout)
{
    printf("SpriteAnimationLoader: Loading animation frames for %s from %s\n",
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

            printf("SpriteAnimationLoader: Extracting frame %d, direction %d at (%d, %d)\n",
                   frame, static_cast<int>(layout.directions[dir]), frame_x, frame_y);

            // Use the existing working PNG extraction method
            CF_Sprite frame_sprite = extractSpriteFrame(png_path, frame_x, frame_y,
                                                        layout.frame_width, layout.frame_height);

            // Assume the sprite creation succeeded if we got here
            frames.push_back(frame_sprite);
            printf("SpriteAnimationLoader: Added frame %zu\n", frames.size());
        }
    }

    printf("SpriteAnimationLoader: Loaded %zu frames for animation %s\n", frames.size(), layout.name.c_str());
    return frames;
}

// Create complete animation from layout (supports multiple layers)
Animation SpriteAnimationLoader::createAnimation(const std::string &name,
                                                 const std::string &png_path,
                                                 const AnimationLayout &layout,
                                                 float frameDelay)
{
    printf("SpriteAnimationLoader: Creating animation %s from %s\n", name.c_str(), png_path.c_str());

    Animation anim;
    anim.name = name;
    anim.looping = true;

    // Load all frames using the existing PNG system
    std::vector<CF_Sprite> sprites = loadAnimationFrames(png_path, layout);

    if (sprites.empty())
    {
        printf("SpriteAnimationLoader: No frames loaded for animation %s\n", name.c_str());
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

    printf("SpriteAnimationLoader: Created animation %s with %zu frames, duration: %.2fms\n",
           name.c_str(), anim.frames.size(), anim.totalDuration);

    return anim;
}

// Create complete animation from layout with multiple layers
Animation SpriteAnimationLoader::createAnimationWithLayers(const std::string &name,
                                                           const std::string &base_path,
                                                           const AnimationLayout &layout,
                                                           float frameDelay)
{
    printf("SpriteAnimationLoader: Creating layered animation %s with %zu layers\n",
           name.c_str(), layout.filenames.size());

    Animation anim;
    anim.name = name;
    anim.looping = true;

    // Load frames from all layers
    std::vector<std::vector<CF_Sprite>> allLayerSprites;

    for (const auto &filename : layout.filenames)
    {
        std::string png_path = base_path + "/" + filename;
        printf("SpriteAnimationLoader: Loading layer from %s\n", png_path.c_str());

        std::vector<CF_Sprite> layerSprites = loadAnimationFrames(png_path, layout);

        if (layerSprites.empty())
        {
            printf("SpriteAnimationLoader: WARNING: No frames loaded from layer %s\n", filename.c_str());
            continue;
        }

        allLayerSprites.push_back(layerSprites);
    }

    if (allLayerSprites.empty())
    {
        printf("SpriteAnimationLoader: ERROR: No layers loaded for animation %s\n", name.c_str());
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

    printf("SpriteAnimationLoader: Created layered animation %s with %zu frames and %zu layers, duration: %.2fms\n",
           name.c_str(), anim.frames.size(), allLayerSprites.size(), anim.totalDuration);

    return anim;
}

// Load multiple animations and create animation table
AnimationTable SpriteAnimationLoader::loadAnimationTable(const std::string &base_path,
                                                         const std::vector<AnimationLayout> &layouts)
{
    printf("SpriteAnimationLoader: Loading animation table from base path: %s\n", base_path.c_str());

    AnimationTable table;

    for (const auto &layout : layouts)
    {
        // Construct base PNG path for this animation
        // PhysFS requires absolute paths starting with /
        // Also, animations are in subdirectories (idle/, walkcycle/)
        std::string anim_base_path = "/" + base_path + "/" + layout.name;

        printf("SpriteAnimationLoader: Loading animation %s from %s with %zu layers\n",
               layout.name.c_str(), anim_base_path.c_str(), layout.filenames.size());

        // Create animation with all layers
        Animation anim = createAnimationWithLayers(layout.name, anim_base_path, layout);

        if (!anim.frames.empty())
        {
            table.addAnimation(layout.name, anim);
            printf("SpriteAnimationLoader: Added animation %s to table\n", layout.name.c_str());
        }
        else
        {
            printf("SpriteAnimationLoader: Failed to create animation %s\n", layout.name.c_str());
        }
    }

    printf("SpriteAnimationLoader: Animation table loaded with %zu animations\n",
           table.getAnimationNames().size());

    return table;
}

// Get (building on first request) the animation table for a character's layers
std::shared_ptr<const AnimationTable> SpriteAnimationLoader::getCharacterAnimationTable(
    const std::string &sheets_root, const std::vector<std::string> &layer_filenames, int tile_size)
{
    if (layer_filenames.empty() || tile_size <= 0)
        return nullptr;

    std::string key = sheets_root + "|" + std::to_string(tile_size);
    for (const auto &fn : layer_filenames)
        key += "|" + fn;

    auto cached = tableCache.find(key);
    if (cached != tableCache.end())
        return cached->second;

    // Discover every animation subfolder (idle, walkcycle, melee_right_claw, etc.) and compute its
    // layout from PNG dimensions, so new animations don't require code changes.
    std::vector<AnimationLayout> layouts;
    const char **entries = cf_fs_enumerate_directory(sheets_root.c_str());
    if (entries)
    {
        for (const char **entry = entries; *entry; ++entry)
        {
            std::string anim_name = *entry;
            std::string folder = sheets_root + "/" + anim_name;

            CF_Stat stat;
            if (cf_is_error(cf_fs_stat(folder.c_str(), &stat)) || stat.type != CF_FILE_TYPE_DIRECTORY)
                continue;

            uint32_t w = 0, h = 0;
            if (!getPNGDimensions(folder + "/" + layer_filenames[0], w, h))
            {
                printf("SpriteAnimationLoader: WARNING: Cannot read dimensions for animation '%s', skipping\n", anim_name.c_str());
                continue;
            }

            layouts.emplace_back(
                anim_name, layer_filenames, tile_size, tile_size, (int)w / tile_size, (int)h / tile_size,
                std::vector<Direction>{Direction::UP, Direction::LEFT, Direction::DOWN, Direction::RIGHT});
        }
        cf_fs_free_enumerated_directory(entries);
    }

    if (layouts.empty())
        return nullptr;

    // loadAnimationTable builds "/" + base_path + "/" + name, so strip the leading slash
    std::string base = sheets_root;
    if (!base.empty() && base.front() == '/')
        base.erase(0, 1);

    auto table = std::make_shared<const AnimationTable>(loadAnimationTable(base, layouts));
    tableCache[key] = table;
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

int Animation::getFrameCount(Direction direction) const
{
    int count = 0;
    for (const auto &frame : frames)
    {
        if (frame.direction == direction)
        {
            count++;
        }
    }
    return count;
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
