#include "SpriteAnimationDemo.h"
#include <cute.h>
#include <cute_draw.h>
#include <cute_math.h>

using namespace Cute;

// Constructor
SpriteAnimationDemo::SpriteAnimationDemo()
    : initialized(false), demoTime(0.0f), directionChangeTime(0.0f), animationChangeTime(0.0f),
      currentAnimation("idle"), currentDirection(Direction::DOWN), currentFrame(0), frameTimer(0.0f),
      position(v2(0, 0))
{
    // Initialize input state
    for (int i = 0; i < 4; i++) {
        keysPressed[i] = false;
    }
    for (int i = 0; i < 2; i++) {
        animationKeys[i] = false;
    }
}

// Destructor
SpriteAnimationDemo::~SpriteAnimationDemo() {
    // Cleanup is automatic
}

// Initialize the demo with skeleton animations
bool SpriteAnimationDemo::init() {
    // Define the animation layouts for skeleton animations
    std::vector<AnimationLayout> layouts = {
        AnimationLayouts::IDLE_4_DIRECTIONS,
        AnimationLayouts::WALKCYCLE_4_DIRECTIONS_9_FRAMES
    };

    // Test file access
    std::string idle_body_path = "assets/Art/AnimationsSheets/idle/BODY_skeleton.png";
    std::string walkcycle_body_path = "assets/Art/AnimationsSheets/walkcycle/BODY_skeleton.png";

    // Test if we can read both body PNG files
    size_t idle_file_size = 0;
    void *idle_file_data = cf_fs_read_entire_file_to_memory(idle_body_path.c_str(), &idle_file_size);

    if (idle_file_data != nullptr) {
        cf_free(idle_file_data);
    } else {
        printf("SpriteAnimationDemo: ERROR: Cannot read %s\n", idle_body_path.c_str());
        return false;
    }

    // Test walkcycle body
    size_t walkcycle_file_size = 0;
    void *walkcycle_file_data = cf_fs_read_entire_file_to_memory(walkcycle_body_path.c_str(), &walkcycle_file_size);

    if (walkcycle_file_data != nullptr) {
        cf_free(walkcycle_file_data);
    } else {
        printf("SpriteAnimationDemo: ERROR: Cannot read %s\n", walkcycle_body_path.c_str());
        return false;
    }

    // Load the animation table using our new system
    animationTable = loader.loadAnimationTable("assets/Art/AnimationsSheets", layouts);

    if (animationTable.getAnimationNames().empty()) {
        printf("SpriteAnimationDemo: Failed to load animations from skeleton assets\n");
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
void SpriteAnimationDemo::update(float dt) {
    if (!initialized) return;

    demoTime += dt;

    // Handle input
    handleInput();

    // Auto-cycle demo elements - DISABLED for player control
    // Player now has full control via WASD keys and 1/2/SPACE keys
    // No automatic direction or animation changes

    // Update animation
    updateAnimation(dt);
}

// Handle input for demo controls
void SpriteAnimationDemo::handleInput() {
    if (!initialized) return;

    // Direction controls (WASD + Arrow Keys)
    bool prevKeysPressed[4] = {keysPressed[0], keysPressed[1], keysPressed[2], keysPressed[3]};

    keysPressed[0] = key_down(CF_KEY_W) || key_down(CF_KEY_UP);      // UP
    keysPressed[1] = key_down(CF_KEY_A) || key_down(CF_KEY_LEFT);    // LEFT
    keysPressed[2] = key_down(CF_KEY_S) || key_down(CF_KEY_DOWN);    // DOWN
    keysPressed[3] = key_down(CF_KEY_D) || key_down(CF_KEY_RIGHT);   // RIGHT

    // Animation controls (1, 2)
    animationKeys[0] = key_just_pressed(CF_KEY_1);  // Switch to idle
    animationKeys[1] = key_just_pressed(CF_KEY_2);  // Switch to walkcycle

    // Additional controls
    bool spacePressed = key_just_pressed(CF_KEY_SPACE);  // Toggle animation
    bool rPressed = key_just_pressed(CF_KEY_R);          // Reset position

    // Handle direction input
    if (keysPressed[0] && !prevKeysPressed[0]) {
        currentDirection = Direction::UP;
    }
    else if (keysPressed[1] && !prevKeysPressed[1]) {
        currentDirection = Direction::LEFT;
    }
    else if (keysPressed[2] && !prevKeysPressed[2]) {
        currentDirection = Direction::DOWN;
    }
    else if (keysPressed[3] && !prevKeysPressed[3]) {
        currentDirection = Direction::RIGHT;
    }

    // Handle movement with WASD (move the character position)
    float moveSpeed = 100.0f; // pixels per second
    float dt = CF_DELTA_TIME;

    if (keysPressed[0]) position.y += moveSpeed * dt;  // UP
    if (keysPressed[1]) position.x -= moveSpeed * dt;  // LEFT
    if (keysPressed[2]) position.y -= moveSpeed * dt;  // DOWN
    if (keysPressed[3]) position.x += moveSpeed * dt;  // RIGHT

    // Auto-switch to walkcycle when moving, idle when stopping
    bool isMoving = keysPressed[0] || keysPressed[1] || keysPressed[2] || keysPressed[3];
    static bool wasMoving = false;

    if (isMoving && !wasMoving) {
        if (currentAnimation != "walkcycle") {
            currentAnimation = "walkcycle";
            currentFrame = 0;
            frameTimer = 0.0f;
        }
    } else if (!isMoving && wasMoving) {
        if (currentAnimation != "idle") {
            currentAnimation = "idle";
            currentFrame = 0;
            frameTimer = 0.0f;
        }
    }
    wasMoving = isMoving;

    // Handle manual animation input (overrides auto-switching)
    if (animationKeys[0]) {
        currentAnimation = "idle";
        currentFrame = 0;
        frameTimer = 0.0f;
    }
    else if (animationKeys[1]) {
        currentAnimation = "walkcycle";
        currentFrame = 0;
        frameTimer = 0.0f;
    }

    // Handle space bar - toggle between idle and walkcycle
    if (spacePressed) {
        std::string newAnimation = (currentAnimation == "idle") ? "walkcycle" : "idle";
        currentAnimation = newAnimation;
        currentFrame = 0;
        frameTimer = 0.0f;
    }

    // Handle R key - reset position
    if (rPressed) {
        position = v2(0.0f, 0.0f);
    }
}

// Update animation state
void SpriteAnimationDemo::updateAnimation(float dt) {
    const Animation* anim = animationTable.getAnimation(currentAnimation);
    if (!anim || anim->frames.empty()) return;

    // Update frame timer
    frameTimer += dt * 1000.0f; // Convert to milliseconds

    // Find current frame
    const AnimationFrame* currentAnimFrame = nullptr;
    for (const auto& frame : anim->frames) {
        if (frame.direction == currentDirection && frame.frameIndex == currentFrame) {
            currentAnimFrame = &frame;
            break;
        }
    }

    // If we found a frame, check if we should advance
    if (currentAnimFrame && frameTimer >= currentAnimFrame->delay) {
        frameTimer = 0.0f;

        // Advance to next frame - but handle idle vs animated differently
        if (currentAnimation == "idle") {
            // Idle animations don't advance frames - they stay at frame 0 for each direction
            currentFrame = 0;
        } else {
            // Walkcycle and other animations advance through frames within each direction
            currentFrame++;

            // For walkcycle: 9 frames per direction, so max frame index is 8
            int maxFramesPerDirection = 9;
            if (currentFrame >= maxFramesPerDirection) {
                currentFrame = 0;
            }
        }
    }
}

// Cycle through directions
void SpriteAnimationDemo::cycleDirection() {
    int currentDir = static_cast<int>(currentDirection);
    currentDir = (currentDir + 1) % 4;
    currentDirection = static_cast<Direction>(currentDir);
}

// Cycle through animations
void SpriteAnimationDemo::cycleAnimation() {
    if (currentAnimation == "idle") {
        currentAnimation = "walkcycle";
    } else {
        currentAnimation = "idle";
    }

    currentFrame = 0;
    frameTimer = 0.0f;
}

// Render the demo
void SpriteAnimationDemo::render() {
    if (!initialized) return;

    renderCurrentFrame();
    renderDebugInfo();
}

// Render the demo at a specific position
void SpriteAnimationDemo::render(v2 renderPosition) {
    if (!initialized) return;

    renderCurrentFrameAt(renderPosition);
    renderDebugInfo();
}

// Render the current animation frame
void SpriteAnimationDemo::renderCurrentFrame() {
    const Animation* anim = animationTable.getAnimation(currentAnimation);
    if (!anim || anim->frames.empty()) return;

    // Find the current frame for the current direction
    const AnimationFrame* currentAnimFrame = nullptr;
    for (const auto& frame : anim->frames) {
        if (frame.direction == currentDirection && frame.frameIndex == currentFrame) {
            currentAnimFrame = &frame;
            break;
        }
    }

    if (!currentAnimFrame) return;
    if (currentAnimFrame->sprite.w <= 0 || currentAnimFrame->sprite.h <= 0) return;

    // Render the sprite
    cf_draw_sprite(&currentAnimFrame->sprite);
}

// Render the current animation frame at a specific position
void SpriteAnimationDemo::renderCurrentFrameAt(v2 renderPosition) {
    const Animation* anim = animationTable.getAnimation(currentAnimation);
    if (!anim || anim->frames.empty()) return;

    // Find the current frame for the current direction
    const AnimationFrame* currentAnimFrame = nullptr;
    for (const auto& frame : anim->frames) {
        if (frame.direction == currentDirection && frame.frameIndex == currentFrame) {
            currentAnimFrame = &frame;
            break;
        }
    }

    if (!currentAnimFrame) return;
    if (currentAnimFrame->sprite.w <= 0 || currentAnimFrame->sprite.h <= 0) return;

    // Apply position transformation and render sprite
    cf_draw_push();
    cf_draw_translate_v2(renderPosition);
    cf_draw_sprite(&currentAnimFrame->sprite);
    cf_draw_pop();
}

// Render debug information
void SpriteAnimationDemo::renderDebugInfo() {
    cf_draw_push_color(make_color(1.0f, 1.0f, 1.0f, 1.0f));

    // Render demo info
    v2 textPos = v2(-600, 300);

    draw_text("SpriteAnimationDemo - Skeleton Animations", textPos);

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
    const char* directionNames[] = {"UP", "LEFT", "DOWN", "RIGHT"};
    int dirIndex = static_cast<int>(currentDirection);
    const char* dirName = (dirIndex >= 0 && dirIndex < 4) ? directionNames[dirIndex] : "UNKNOWN";
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
bool SpriteAnimationDemo::isValid() const {
    return initialized && !animationTable.getAnimationNames().empty();
}
