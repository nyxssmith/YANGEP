#include "PNGSprite.h"
#include <cute.h>
#include <cute_png_cache.h>
#include <cute_sprite.h>
#include <cute_draw.h>
#include <cute_math.h>
#include <cstdlib>

using namespace Cute;

// Default constructor
PNGSprite::PNGSprite()
    : idleAnimationTable(nullptr), walkcycleAnimationTable(nullptr),
      idleSprite({0}), walkcycleSprite({0}),
      currentAnimation("idle"), currentDirection(Direction::DOWN),
      position(v2(0, 0)), scale(v2(1, 1)), rotation(0.0f)
{
}

// Destructor
PNGSprite::~PNGSprite() {
    cleanup();
}

// Load both idle and walkcycle animations using sprite sheet splitters
bool PNGSprite::loadAnimations(const char* idle_path, const char* walkcycle_path) {
    if (!idle_path || !walkcycle_path) {
        printf("FATAL ERROR: Invalid animation paths provided\n");
        exit(1);
    }

    // Clean up any existing data
    cleanup();

    // Load idle sprite sheet (64x256: 1 frame per direction)
    if (!idleSplitter.loadSpriteSheet(idle_path, 64, 64, 1)) {
        printf("FATAL ERROR: Failed to load idle sprite sheet\n");
        exit(1);
    }

    // Load walkcycle sprite sheet (576x256: 9 frames per direction)
    if (!walkcycleSplitter.loadSpriteSheet(walkcycle_path, 64, 64, 9)) {
        printf("FATAL ERROR: Failed to load walkcycle sprite sheet\n");
        exit(1);
    }

    printf("Successfully loaded both sprite sheets using splitters\n");

    // Create animation tables using virtual PNGs
    if (!createIdleAnimationTable()) {
        printf("FATAL ERROR: Failed to create idle animation table\n");
        exit(1);
    }

    if (!createWalkcycleAnimationTable()) {
        printf("FATAL ERROR: Failed to create walkcycle animation table\n");
        exit(1);
    }

    return true;
}

// Create idle animation table using virtual PNGs
bool PNGSprite::createIdleAnimationTable() {
    if (!idleSplitter.isReady()) return false;

    // Create individual animations for each direction using virtual PNGs
    float idle_delay = 100.0f;  // 100ms per frame

    // Get individual frames for each direction
    VirtualPNG up_frame = idleSplitter.getFramePNG(Direction::UP);
    VirtualPNG left_frame = idleSplitter.getFramePNG(Direction::LEFT);
    VirtualPNG down_frame = idleSplitter.getFramePNG(Direction::DOWN);
    VirtualPNG right_frame = idleSplitter.getFramePNG(Direction::RIGHT);

    const CF_Animation* idle_up = cf_make_png_cache_animation(
        "idle_up",
        &up_frame.png,
        1,
        &idle_delay,
        1
    );
    const CF_Animation* idle_left = cf_make_png_cache_animation(
        "idle_left",
        &left_frame.png,
        1,
        &idle_delay,
        1
    );
    const CF_Animation* idle_down = cf_make_png_cache_animation(
        "idle_down",
        &down_frame.png,
        1,
        &idle_delay,
        1
    );
    const CF_Animation* idle_right = cf_make_png_cache_animation(
        "idle_right",
        &right_frame.png,
        1,
        &idle_delay,
        1
    );

    if (!idle_up || !idle_left || !idle_down || !idle_right) {
        printf("Failed to create idle animations\n");
        return false;
    }

    // Create animation table
    const CF_Animation* idle_anims[] = {idle_up, idle_left, idle_down, idle_right};
    idleAnimationTable = cf_make_png_cache_animation_table(
        "skeleton_idle", idle_anims, 4
    );

    if (!idleAnimationTable) {
        printf("Failed to create idle animation table\n");
        return false;
    }

    // Create sprite for idle animations
    idleSprite = cf_make_png_cache_sprite("skeleton_idle", idleAnimationTable);
    if (idleSprite.w == 0 || idleSprite.h == 0) {
        printf("Failed to create idle sprite\n");
        return false;
    }

    printf("DEBUG: Idle sprite created - w: %d, h: %d, animations: %p, animation: %p\n",
           idleSprite.w, idleSprite.h, idleSprite.animations, idleSprite.animation);

    // Set initial animation to avoid CF_ASSERT failures
    cf_sprite_play(&idleSprite, "idle_down");

    printf("DEBUG: After cf_sprite_play - animation: %p\n", idleSprite.animation);

    // IMMEDIATE VALIDATION: Try to trigger CF_ASSERT failures now
    printf("DEBUG: Testing idle sprite validation...\n");
    printf("DEBUG: idleSprite.animations: %p\n", idleSprite.animations);
    printf("DEBUG: idleSprite.animation: %p\n", idleSprite.animation);

    // Try to access sprite properties that should trigger validation
    if (idleSprite.animation) {
        printf("DEBUG: idleSprite.animation->name: %s\n", idleSprite.animation->name);
    } else {
        printf("DEBUG: idleSprite.animation is NULL - this should trigger CF_ASSERT!\n");
    }

    // Force a sprite update to trigger validation
    printf("DEBUG: Calling cf_sprite_update on idle sprite...\n");
    cf_sprite_update(&idleSprite);

    printf("Successfully created idle animation table with 4 animations\n");
    return true;
}

// Create walkcycle animation table using virtual PNGs
bool PNGSprite::createWalkcycleAnimationTable() {
    if (!walkcycleSplitter.isReady()) return false;

    // Create individual animations for each direction using virtual PNGs
    float walk_delays[] = {80.0f, 80.0f, 80.0f, 80.0f, 80.0f, 80.0f, 80.0f, 80.0f, 80.0f};  // 80ms per frame

    // Get all frames for each direction
    std::vector<VirtualPNG> up_frames = walkcycleSplitter.getAllFramesForDirection(Direction::UP);
    std::vector<VirtualPNG> left_frames = walkcycleSplitter.getAllFramesForDirection(Direction::LEFT);
    std::vector<VirtualPNG> down_frames = walkcycleSplitter.getAllFramesForDirection(Direction::DOWN);
    std::vector<VirtualPNG> right_frames = walkcycleSplitter.getAllFramesForDirection(Direction::RIGHT);

    // Convert VirtualPNG to CF_Png array for each direction
    std::vector<CF_Png> up_pngs, left_pngs, down_pngs, right_pngs;
    for (const auto& frame : up_frames) up_pngs.push_back(frame.png);
    for (const auto& frame : left_frames) left_pngs.push_back(frame.png);
    for (const auto& frame : down_frames) down_pngs.push_back(frame.png);
    for (const auto& frame : right_frames) right_pngs.push_back(frame.png);

    const CF_Animation* walk_up = cf_make_png_cache_animation(
        "walk_up",
        up_pngs.data(),
        9,
        walk_delays,
        9
    );
    const CF_Animation* walk_left = cf_make_png_cache_animation(
        "walk_left",
        left_pngs.data(),
        9,
        walk_delays,
        9
    );
    const CF_Animation* walk_down = cf_make_png_cache_animation(
        "walk_down",
        down_pngs.data(),
        9,
        walk_delays,
        9
    );
    const CF_Animation* walk_right = cf_make_png_cache_animation(
        "walk_right",
        right_pngs.data(),
        9,
        walk_delays,
        9
    );

    if (!walk_up || !walk_left || !walk_down || !walk_right) {
        printf("Failed to create walkcycle animations\n");
        return false;
    }

    // Create animation table
    const CF_Animation* walk_anims[] = {walk_up, walk_left, walk_down, walk_right};
    walkcycleAnimationTable = cf_make_png_cache_animation_table(
        "skeleton_walk", walk_anims, 4
    );

    if (!walkcycleAnimationTable) {
        printf("Failed to create walkcycle animation table\n");
        return false;
    }

    // Create sprite for walkcycle animations
    walkcycleSprite = cf_make_png_cache_sprite("skeleton_walk", walkcycleAnimationTable);
    if (walkcycleSprite.w == 0 || walkcycleSprite.h == 0) {
        printf("Failed to create walkcycle sprite\n");
        return false;
    }

    printf("DEBUG: Walkcycle sprite created - w: %d, h: %d, animations: %p, animation: %p\n",
           walkcycleSprite.w, walkcycleSprite.h, walkcycleSprite.animations, walkcycleSprite.animation);

    // Set initial animation to avoid CF_ASSERT failures
    cf_sprite_play(&walkcycleSprite, "walk_down");

    printf("DEBUG: After cf_sprite_play - animation: %p\n", walkcycleSprite.animation);

    // IMMEDIATE VALIDATION: Try to trigger CF_ASSERT failures now
    printf("DEBUG: Testing walkcycle sprite validation...\n");
    printf("DEBUG: walkcycleSprite.animations: %p\n", walkcycleSprite.animations);
    printf("DEBUG: walkcycleSprite.animation: %p\n", walkcycleSprite.animation);

    // Try to access sprite properties that should trigger validation
    if (walkcycleSprite.animation) {
        printf("DEBUG: walkcycleSprite.animation->name: %s\n", walkcycleSprite.animation->name);
    } else {
        printf("DEBUG: walkcycleSprite.animation is NULL - this should trigger CF_ASSERT!\n");
    }

    // Force a sprite update to trigger validation
    printf("DEBUG: Calling cf_sprite_update on walkcycle sprite...\n");
    cf_sprite_update(&walkcycleSprite);

    printf("Successfully created walkcycle animation table with 4 animations\n");
    return true;
}

// Render the sprite
void PNGSprite::render() {
    if (!isValid()) return;

    // Save current draw state
    cf_draw_push();

    // Apply transforms
    cf_draw_translate_v2(position);
    cf_draw_scale_v2(scale);
    cf_draw_rotate(rotation);

    // Render based on current animation
    if (strcmp(currentAnimation, "idle") == 0) {
        cf_draw_sprite(&idleSprite);
    } else {
        cf_draw_sprite(&walkcycleSprite);
    }

    // Debug: Draw a colored border to show frame bounds
    cf_draw_push_color(make_color(0.0f, 1.0f, 0.0f, 1.0f)); // Green border

    // Create frame bounds for debug visualization
    v2 frameSize = v2(64.0f, 64.0f) * scale;
    v2 framePos = v2(-frameSize.x * 0.5f, -frameSize.y * 0.5f);

    // Draw wireframe around frame bounds
    CF_Aabb frameAABB;
    frameAABB.min = framePos;
    frameAABB.max = framePos + frameSize;
    cf_draw_quad(frameAABB, 2.0f, 0.0f);

    cf_draw_pop_color();

    // Debug output
    printf("DEBUG: Rendering %s animation, direction %d\n",
           currentAnimation, static_cast<int>(currentDirection));

    // Restore draw state
    cf_draw_pop();
}

// Update sprite state
void PNGSprite::update(float dt) {
    if (!isValid()) return;

    // Update sprite animations
    cf_sprite_update(&idleSprite);
    cf_sprite_update(&walkcycleSprite);
}

// Set direction
void PNGSprite::setDirection(Direction direction) {
    currentDirection = direction;

    // Update both sprites to play the correct direction
    const char* direction_names[] = {"up", "left", "down", "right"};
    const char* current_dir = direction_names[static_cast<int>(direction)];

    if (idleSplitter.isReady()) {
        cf_sprite_play(&idleSprite, current_dir);
    }
    if (walkcycleSplitter.isReady()) {
        cf_sprite_play(&walkcycleSprite, current_dir);
    }

    printf("Direction changed to: %d (%s)\n", static_cast<int>(direction), current_dir);
}

// Get current direction
Direction PNGSprite::getDirection() const {
    return currentDirection;
}

// Set animation
void PNGSprite::setAnimation(const char* animation_name) {
    if (!animation_name || !isValid()) return;

    currentAnimation = animation_name;
    printf("Animation changed to: %s\n", animation_name);
}

// Get current animation
const char* PNGSprite::getCurrentAnimation() const {
    return currentAnimation;
}

// Set position
void PNGSprite::setPosition(v2 pos) {
    position = pos;
}

// Set scale
void PNGSprite::setScale(v2 s) {
    scale = s;
}

// Set rotation
void PNGSprite::setRotation(float rot) {
    rotation = rot;
}

// Get position
v2 PNGSprite::getPosition() const {
    return position;
}

// Get scale
v2 PNGSprite::getScale() const {
    return scale;
}

// Get rotation
float PNGSprite::getRotation() const {
    return rotation;
}

// Check if sprite is valid
bool PNGSprite::isValid() const {
    return idleSplitter.isReady() && walkcycleSplitter.isReady() &&
           idleSprite.w > 0 && idleSprite.h > 0 &&
           walkcycleSprite.w > 0 && walkcycleSprite.h > 0;
}

// Get sprite width
int PNGSprite::getWidth() const {
    return 64;  // Each frame is 64x64
}

// Get sprite height
int PNGSprite::getHeight() const {
    return 64;  // Each frame is 64x64
}

// Cleanup resources
void PNGSprite::cleanup() {
    idleSplitter.unloadSpriteSheet();
    walkcycleSplitter.unloadSpriteSheet();

    // Note: Cute Framework handles sprite and animation table cleanup automatically
    // We don't need to manually destroy these resources
}
