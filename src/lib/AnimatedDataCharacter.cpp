#include "AnimatedDataCharacter.h"
#include "DataFile.h"
#include <cute.h>
#include <cute_draw.h>
#include <cute_math.h>
#include <spng.h>

using namespace Cute;

// Helper function to get PNG dimensions
static bool getPNGDimensions(const std::string &path, uint32_t &width, uint32_t &height)
{
    size_t file_size = 0;
    void *file_data = cf_fs_read_entire_file_to_memory(path.c_str(), &file_size);

    if (!file_data)
    {
        return false;
    }

    spng_ctx *ctx = spng_ctx_new(0);
    if (!ctx)
    {
        cf_free(file_data);
        return false;
    }

    spng_set_png_buffer(ctx, file_data, file_size);

    struct spng_ihdr ihdr;
    int ret = spng_get_ihdr(ctx, &ihdr);

    if (ret == 0)
    {
        width = ihdr.width;
        height = ihdr.height;
    }

    spng_ctx_free(ctx);
    cf_free(file_data);

    return ret == 0;
}

// Constructor
AnimatedDataCharacter::AnimatedDataCharacter()
    : initialized(false), demoTime(0.0f), directionChangeTime(0.0f), animationChangeTime(0.0f),
      currentAnimation("idle"), currentDirection(Direction::DOWN), currentFrame(0), frameTimer(0.0f),
      position(v2(0, 0))
{
    // Initialize input state
    for (int i = 0; i < 4; i++)
    {
        keysPressed[i] = false;
    }
    for (int i = 0; i < 2; i++)
    {
        animationKeys[i] = false;
    }
}

// Destructor
AnimatedDataCharacter::~AnimatedDataCharacter()
{
    // Cleanup is automatic
}

// Initialize the character with a datafile path
bool AnimatedDataCharacter::init(const std::string &datafilePath)
{
    // Load the datafile
    if (!datafile.load(datafilePath))
    {
        printf("AnimatedDataCharacter: ERROR: Failed to load datafile from %s\n", datafilePath.c_str());
        return false;
    }

    // Validate datafile structure
    if (!datafile.contains("character_config"))
    {
        printf("AnimatedDataCharacter: ERROR: Datafile missing 'character_config' section\n");
        return false;
    }

    auto &charConfig = datafile["character_config"];

    if (!charConfig.contains("name") || !charConfig.contains("layers"))
    {
        printf("AnimatedDataCharacter: ERROR: character_config missing required fields\n");
        return false;
    }

    std::string characterName = charConfig["name"];
    printf("AnimatedDataCharacter: Loading character '%s' from datafile\n", characterName.c_str());

    // Get the first layer from the layers array
    if (!charConfig["layers"].is_array() || charConfig["layers"].empty())
    {
        printf("AnimatedDataCharacter: ERROR: character_config.layers is empty or not an array\n");
        return false;
    }

    auto &firstLayer = charConfig["layers"][0];

    if (!firstLayer.contains("filename"))
    {
        printf("AnimatedDataCharacter: ERROR: First layer missing 'filename' field\n");
        return false;
    }

    if (!firstLayer.contains("tile_size"))
    {
        printf("AnimatedDataCharacter: ERROR: First layer missing 'tile_size' field\n");
        return false;
    }

    std::string layerFilename = firstLayer["filename"];
    int tileSize = firstLayer["tile_size"];

    printf("AnimatedDataCharacter: Using layer filename: %s\n", layerFilename.c_str());
    printf("AnimatedDataCharacter: Using tile size: %d\n", tileSize);

    // Construct paths using the layer filename from the datafile
    std::string idle_body_path = "assets/Art/AnimationsSheets/idle/" + layerFilename;
    std::string walkcycle_body_path = "assets/Art/AnimationsSheets/walkcycle/" + layerFilename;

    // Get dimensions for idle animation
    uint32_t idle_width = 0, idle_height = 0;
    if (!getPNGDimensions(idle_body_path, idle_width, idle_height))
    {
        printf("AnimatedDataCharacter: ERROR: Cannot read dimensions from %s\n", idle_body_path.c_str());
        return false;
    }

    // Get dimensions for walkcycle animation
    uint32_t walkcycle_width = 0, walkcycle_height = 0;
    if (!getPNGDimensions(walkcycle_body_path, walkcycle_width, walkcycle_height))
    {
        printf("AnimatedDataCharacter: ERROR: Cannot read dimensions from %s\n", walkcycle_body_path.c_str());
        return false;
    }

    // Calculate frame counts dynamically
    // Frames per direction = image_width / tile_size
    // Number of directions = image_height / tile_size
    int idle_frames_per_direction = idle_width / tileSize;
    int idle_direction_count = idle_height / tileSize;

    int walkcycle_frames_per_direction = walkcycle_width / tileSize;
    int walkcycle_direction_count = walkcycle_height / tileSize;

    printf("AnimatedDataCharacter: Idle dimensions: %ux%u (frames: %d, directions: %d)\n",
           idle_width, idle_height, idle_frames_per_direction, idle_direction_count);
    printf("AnimatedDataCharacter: Walkcycle dimensions: %ux%u (frames: %d, directions: %d)\n",
           walkcycle_width, walkcycle_height, walkcycle_frames_per_direction, walkcycle_direction_count);

    // Define the animation layouts using computed values
    std::vector<AnimationLayout> layouts = {
        AnimationLayout(
            "idle", tileSize, tileSize, idle_frames_per_direction, idle_direction_count,
            {Direction::UP, Direction::LEFT, Direction::DOWN, Direction::RIGHT}),
        AnimationLayout(
            "walkcycle", tileSize, tileSize, walkcycle_frames_per_direction, walkcycle_direction_count,
            {Direction::UP, Direction::LEFT, Direction::DOWN, Direction::RIGHT})};

    // Load the animation table using our new system
    animationTable = loader.loadAnimationTable("assets/Art/AnimationsSheets", layouts);

    if (animationTable.getAnimationNames().empty())
    {
        printf("AnimatedDataCharacter: Failed to load animations from skeleton assets\n");
        return false;
    }

    // Set initial state
    currentAnimation = "idle";
    currentDirection = Direction::DOWN;
    currentFrame = 0;
    frameTimer = 0.0f;

    initialized = true;
    return true;
}

// Update demo state
void AnimatedDataCharacter::update(float dt, v2 moveVector)
{
    if (!initialized)
        return;

    demoTime += dt;

    // Calculate movement magnitude to determine if we're moving
    float moveMagnitude = sqrt(moveVector.x * moveVector.x + moveVector.y * moveVector.y);
    bool isMoving = moveMagnitude > 0.01f; // Small threshold to avoid floating point issues

    // Update direction based on move vector
    if (isMoving)
    {
        // Determine direction based on the dominant axis
        if (cf_abs(moveVector.x) > cf_abs(moveVector.y))
        {
            // Horizontal movement is dominant
            currentDirection = (moveVector.x > 0) ? Direction::RIGHT : Direction::LEFT;
        }
        else
        {
            // Vertical movement is dominant
            currentDirection = (moveVector.y > 0) ? Direction::UP : Direction::DOWN;
        }
    }

    // Auto-switch to walkcycle when moving, idle when stopping
    static bool wasMoving = false;

    if (isMoving && !wasMoving)
    {
        if (currentAnimation != "walkcycle")
        {
            currentAnimation = "walkcycle";
            currentFrame = 0;
            frameTimer = 0.0f;
        }
    }
    else if (!isMoving && wasMoving)
    {
        if (currentAnimation != "idle")
        {
            currentAnimation = "idle";
            currentFrame = 0;
            frameTimer = 0.0f;
        }
    }
    wasMoving = isMoving;

    // Handle manual animation input (allows user to override auto-switching with 1/2/SPACE keys)
    handleInput();

    // Apply movement based on move vector
    position.x += moveVector.x * dt;
    position.y += moveVector.y * dt;

    // Update animation
    updateAnimation(dt);
}

// Handle input for demo controls
void AnimatedDataCharacter::handleInput()
{
    if (!initialized)
        return;

    // Animation controls (1, 2) - manual override of auto-switching
    bool animKey1 = key_just_pressed(CF_KEY_1); // Switch to idle
    bool animKey2 = key_just_pressed(CF_KEY_2); // Switch to walkcycle

    // Additional controls
    bool spacePressed = key_just_pressed(CF_KEY_SPACE); // Toggle animation
    bool rPressed = key_just_pressed(CF_KEY_R);         // Reset position

    // Handle manual animation input (overrides auto-switching)
    if (animKey1)
    {
        currentAnimation = "idle";
        currentFrame = 0;
        frameTimer = 0.0f;
    }
    else if (animKey2)
    {
        currentAnimation = "walkcycle";
        currentFrame = 0;
        frameTimer = 0.0f;
    }

    // Handle space bar - toggle between idle and walkcycle
    if (spacePressed)
    {
        std::string newAnimation = (currentAnimation == "idle") ? "walkcycle" : "idle";
        currentAnimation = newAnimation;
        currentFrame = 0;
        frameTimer = 0.0f;
    }

    // Handle R key - reset position
    if (rPressed)
    {
        position = v2(0.0f, 0.0f);
    }
}

// Update animation state
void AnimatedDataCharacter::updateAnimation(float dt)
{
    const Animation *anim = animationTable.getAnimation(currentAnimation);
    if (!anim || anim->frames.empty())
        return;

    // Update frame timer
    frameTimer += dt * 1000.0f; // Convert to milliseconds

    // Find current frame
    const AnimationFrame *currentAnimFrame = nullptr;
    for (const auto &frame : anim->frames)
    {
        if (frame.direction == currentDirection && frame.frameIndex == currentFrame)
        {
            currentAnimFrame = &frame;
            break;
        }
    }

    // If we found a frame, check if we should advance
    if (currentAnimFrame && frameTimer >= currentAnimFrame->delay)
    {
        frameTimer = 0.0f;

        // Advance to next frame - but handle idle vs animated differently
        if (currentAnimation == "idle")
        {
            // Idle animations don't advance frames - they stay at frame 0 for each direction
            currentFrame = 0;
        }
        else
        {
            // Walkcycle and other animations advance through frames within each direction
            currentFrame++;

            // For walkcycle: 9 frames per direction, so max frame index is 8
            int maxFramesPerDirection = 9;
            if (currentFrame >= maxFramesPerDirection)
            {
                currentFrame = 0;
            }
        }
    }
}

// Cycle through directions
void AnimatedDataCharacter::cycleDirection()
{
    int currentDir = static_cast<int>(currentDirection);
    currentDir = (currentDir + 1) % 4;
    currentDirection = static_cast<Direction>(currentDir);
}

// Cycle through animations
void AnimatedDataCharacter::cycleAnimation()
{
    if (currentAnimation == "idle")
    {
        currentAnimation = "walkcycle";
    }
    else
    {
        currentAnimation = "idle";
    }

    currentFrame = 0;
    frameTimer = 0.0f;
}

// Render the demo
void AnimatedDataCharacter::render()
{
    if (!initialized)
        return;

    renderCurrentFrame();
    renderDebugInfo();
}

// Render the demo at a specific position
void AnimatedDataCharacter::render(v2 renderPosition)
{
    if (!initialized)
        return;

    renderCurrentFrameAt(renderPosition);
    renderDebugInfo();
}

// Render the current animation frame
void AnimatedDataCharacter::renderCurrentFrame()
{
    const Animation *anim = animationTable.getAnimation(currentAnimation);
    if (!anim || anim->frames.empty())
        return;

    // Find the current frame for the current direction
    const AnimationFrame *currentAnimFrame = nullptr;
    for (const auto &frame : anim->frames)
    {
        if (frame.direction == currentDirection && frame.frameIndex == currentFrame)
        {
            currentAnimFrame = &frame;
            break;
        }
    }

    if (!currentAnimFrame)
        return;
    if (currentAnimFrame->sprite.w <= 0 || currentAnimFrame->sprite.h <= 0)
        return;

    // Render the sprite
    cf_draw_sprite(&currentAnimFrame->sprite);
}

// Render the current animation frame at a specific position
void AnimatedDataCharacter::renderCurrentFrameAt(v2 renderPosition)
{
    const Animation *anim = animationTable.getAnimation(currentAnimation);
    if (!anim || anim->frames.empty())
        return;

    // Find the current frame for the current direction
    const AnimationFrame *currentAnimFrame = nullptr;
    for (const auto &frame : anim->frames)
    {
        if (frame.direction == currentDirection && frame.frameIndex == currentFrame)
        {
            currentAnimFrame = &frame;
            break;
        }
    }

    if (!currentAnimFrame)
        return;
    if (currentAnimFrame->sprite.w <= 0 || currentAnimFrame->sprite.h <= 0)
        return;

    // Apply position transformation and render sprite
    cf_draw_push();
    cf_draw_translate_v2(renderPosition);
    cf_draw_sprite(&currentAnimFrame->sprite);
    cf_draw_pop();
}

// Render debug information
void AnimatedDataCharacter::renderDebugInfo()
{
    cf_draw_push_color(make_color(1.0f, 1.0f, 1.0f, 1.0f));

    // Render demo info
    v2 textPos = v2(-600, 300);

    draw_text("AnimatedDataCharacter - Skeleton Animations", textPos);

    textPos.y -= 30;
    draw_text("Controls:", textPos);

    textPos.y -= 20;
    draw_text("WASD/Arrow Keys: Change direction", textPos);

    textPos.y -= 20;
    draw_text("1: Switch to idle animation", textPos);

    textPos.y -= 20;
    draw_text("2: Switch to walkcycle animation", textPos);

    textPos.y -= 30;
    draw_text("Current State:", textPos);

    char stateText[256];

    textPos.y -= 20;
    snprintf(stateText, sizeof(stateText), "Animation: %s", currentAnimation.c_str());
    draw_text(stateText, textPos);

    textPos.y -= 20;
    const char *directionNames[] = {"UP", "LEFT", "DOWN", "RIGHT"};
    int dirIndex = static_cast<int>(currentDirection);
    const char *dirName = (dirIndex >= 0 && dirIndex < 4) ? directionNames[dirIndex] : "UNKNOWN";
    snprintf(stateText, sizeof(stateText), "Direction: %s (%d)", dirName, dirIndex);
    draw_text(stateText, textPos);

    textPos.y -= 20;
    snprintf(stateText, sizeof(stateText), "Frame: %d", currentFrame);
    draw_text(stateText, textPos);

    textPos.y -= 20;
    snprintf(stateText, sizeof(stateText), "Position: (%.1f, %.1f)", position.x, position.y);
    draw_text(stateText, textPos);

    cf_draw_pop_color();
}

// Check if demo is valid
bool AnimatedDataCharacter::isValid() const
{
    return initialized && !animationTable.getAnimationNames().empty();
}

// Get current position
v2 AnimatedDataCharacter::getPosition() const
{
    return position;
}

void AnimatedDataCharacter::setPosition(v2 newPosition)
{
    position = newPosition;
}
