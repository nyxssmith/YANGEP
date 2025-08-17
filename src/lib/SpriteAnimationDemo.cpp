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
    printf("SpriteAnimationDemo: Initializing with skeleton animations...\n");

    // Define the animation layouts for skeleton animations
    std::vector<AnimationLayout> layouts = {
        AnimationLayouts::IDLE_4_DIRECTIONS,
        AnimationLayouts::WALKCYCLE_4_DIRECTIONS_9_FRAMES
    };

        // Load all animations from the skeleton assets directory
    // Note: We need to create the actual PNG files that match our layouts
    // For now, let's try to load from the existing structure

    printf("SpriteAnimationDemo: Attempting to load skeleton animations...\n");

    // First, let's check what files actually exist
    printf("SpriteAnimationDemo: Checking for skeleton assets...\n");

        // Focus only on skeleton body parts (ignore head pieces for now)
    std::string idle_body_path = "assets/Art/AnimationsSheets/idle/BODY_skeleton.png";
    std::string walkcycle_body_path = "assets/Art/AnimationsSheets/walkcycle/BODY_skeleton.png";

    printf("SpriteAnimationDemo: Looking for skeleton body assets:\n");
    printf("  - Idle body: %s\n", idle_body_path.c_str());
    printf("  - Walkcycle body: %s\n", walkcycle_body_path.c_str());

    // The SpriteAnimationLoader can now work directly with source sprite sheets!

        printf("SpriteAnimationDemo: Loading directly from source sprite sheets\n");
    printf("SpriteAnimationDemo: Source files:\n");
    printf("  - assets/Art/AnimationsSheets/idle/BODY_skeleton.png\n");
    printf("  - assets/Art/AnimationsSheets/walkcycle/BODY_skeleton.png\n");

    // Let's check if we can at least read the individual files
    printf("SpriteAnimationDemo: Testing file access...\n");

    // Test if we can read both body PNG files
    size_t idle_file_size = 0;
    void *idle_file_data = cf_fs_read_entire_file_to_memory(idle_body_path.c_str(), &idle_file_size);

    if (idle_file_data != nullptr) {
        printf("SpriteAnimationDemo: SUCCESS: Can read %s (%zu bytes)\n", idle_body_path.c_str(), idle_file_size);
        cf_free(idle_file_data);
    } else {
        printf("SpriteAnimationDemo: ERROR: Cannot read %s\n", idle_body_path.c_str());
        return false;
    }

    // Test walkcycle body
    size_t walkcycle_file_size = 0;
    void *walkcycle_file_data = cf_fs_read_entire_file_to_memory(walkcycle_body_path.c_str(), &walkcycle_file_size);

    if (walkcycle_file_data != nullptr) {
        printf("SpriteAnimationDemo: SUCCESS: Can read %s (%zu bytes)\n", walkcycle_body_path.c_str(), walkcycle_file_size);
        cf_free(walkcycle_file_data);
    } else {
        printf("SpriteAnimationDemo: ERROR: Cannot read %s\n", walkcycle_body_path.c_str());
        return false;
    }

        printf("SpriteAnimationDemo: Both body PNGs accessible! Ready for animation system.\n");

    // Load animations directly from source files (no combiner needed!)
    printf("SpriteAnimationDemo: Loading animations directly from source files...\n");

    // Load the animation table using our new system
    animationTable = loader.loadAnimationTable("assets/Art/AnimationsSheets", layouts);

    if (animationTable.getAnimationNames().empty()) {
        printf("SpriteAnimationDemo: Failed to load any animations!\n");
        printf("SpriteAnimationDemo: Check if the source PNG files exist at:\n");
        printf("  - assets/Art/AnimationsSheets/idle/BODY_skeleton.png\n");
        printf("  - assets/Art/AnimationsSheets/walkcycle/BODY_skeleton.png\n");
        return false;
    }

    printf("SpriteAnimationDemo: Successfully loaded %zu animations:\n",
           animationTable.getAnimationNames().size());

    for (const auto& name : animationTable.getAnimationNames()) {
        const Animation* anim = animationTable.getAnimation(name);
        if (anim) {
            printf("  - %s: %zu frames, duration: %.2fms\n",
                   name.c_str(), anim->frames.size(), anim->totalDuration);
        }
    }



    // Set initial state (placeholder for now)
    currentAnimation = "idle";
    currentDirection = Direction::DOWN;
    currentFrame = 0;
    frameTimer = 0.0f;

    // Set demo timing
    directionChangeTime = 3.0f;  // Change direction every 3 seconds
    animationChangeTime = 5.0f;  // Change animation every 5 seconds

    initialized = true;
    printf("SpriteAnimationDemo: Initialization complete! Ready for animation playback.\n");

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
    if (!initialized) {
        printf("SpriteAnimationDemo: handleInput() called but not initialized!\n");
        return;
    }

    try {
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

        // Handle direction input with debug output
        Direction prevDirection = currentDirection;

        if (keysPressed[0] && !prevKeysPressed[0]) {
            currentDirection = Direction::UP;
            printf("SpriteAnimationDemo: Direction changed to UP\n");
        }
        else if (keysPressed[1] && !prevKeysPressed[1]) {
            currentDirection = Direction::LEFT;
            printf("SpriteAnimationDemo: Direction changed to LEFT\n");
        }
        else if (keysPressed[2] && !prevKeysPressed[2]) {
            currentDirection = Direction::DOWN;
            printf("SpriteAnimationDemo: Direction changed to DOWN\n");
        }
        else if (keysPressed[3] && !prevKeysPressed[3]) {
            currentDirection = Direction::RIGHT;
            printf("SpriteAnimationDemo: Direction changed to RIGHT\n");
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
                printf("SpriteAnimationDemo: Auto-switched to walkcycle (moving)\n");
            }
        } else if (!isMoving && wasMoving) {
            if (currentAnimation != "idle") {
                currentAnimation = "idle";
                currentFrame = 0;
                frameTimer = 0.0f;
                printf("SpriteAnimationDemo: Auto-switched to idle (stopped moving)\n");
            }
        }
        wasMoving = isMoving;

        // Handle manual animation input (overrides auto-switching)
        if (animationKeys[0]) {
            currentAnimation = "idle";
            currentFrame = 0;
            frameTimer = 0.0f;
            printf("SpriteAnimationDemo: Manual switch to idle animation\n");
        }
        else if (animationKeys[1]) {
            currentAnimation = "walkcycle";
            currentFrame = 0;
            frameTimer = 0.0f;
            printf("SpriteAnimationDemo: Manual switch to walkcycle animation\n");
        }

        // Handle space bar - toggle between idle and walkcycle
        if (spacePressed) {
            std::string newAnimation = (currentAnimation == "idle") ? "walkcycle" : "idle";
            currentAnimation = newAnimation;
            currentFrame = 0;
            frameTimer = 0.0f;
            printf("SpriteAnimationDemo: Toggled to %s animation\n", currentAnimation.c_str());
        }

        // Handle R key - reset position
        if (rPressed) {
            position = v2(0.0f, 0.0f);
            printf("SpriteAnimationDemo: Reset position to (0, 0)\n");
        }

        // Debug output for significant changes
        if (prevDirection != currentDirection) {
            printf("SpriteAnimationDemo: Direction change detected: %d -> %d\n",
                   static_cast<int>(prevDirection), static_cast<int>(currentDirection));
        }

    } catch (const std::exception& e) {
        printf("SpriteAnimationDemo: EXCEPTION in handleInput: %s\n", e.what());
    } catch (...) {
        printf("SpriteAnimationDemo: UNKNOWN EXCEPTION in handleInput\n");
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
            // Only direction changes based on user input, not time
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

        printf("SpriteAnimationDemo: Frame %d, Direction %d, Animation %s\n",
               currentFrame, static_cast<int>(currentDirection), currentAnimation.c_str());
    }
}

// Cycle through directions
void SpriteAnimationDemo::cycleDirection() {
    int currentDir = static_cast<int>(currentDirection);
    currentDir = (currentDir + 1) % 4;
    currentDirection = static_cast<Direction>(currentDir);

    printf("SpriteAnimationDemo: Auto-cycled to direction %d\n", currentDir);
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

    printf("SpriteAnimationDemo: Auto-cycled to animation %s\n", currentAnimation.c_str());
}

// Render the demo
void SpriteAnimationDemo::render() {
    if (!initialized) {
        printf("SpriteAnimationDemo: WARNING - render() called but not initialized!\n");
        return;
    }

    printf("SpriteAnimationDemo: Starting render cycle...\n");

    try {
        // Render current frame with validation
        printf("SpriteAnimationDemo: About to render current frame...\n");
        renderCurrentFrame();
        printf("SpriteAnimationDemo: Current frame rendered successfully\n");

        // Render debug info with validation
        printf("SpriteAnimationDemo: About to render debug info...\n");
        renderDebugInfo();
        printf("SpriteAnimationDemo: Debug info rendered successfully\n");

    } catch (const std::exception& e) {
        printf("SpriteAnimationDemo: EXCEPTION during render: %s\n", e.what());
    } catch (...) {
        printf("SpriteAnimationDemo: UNKNOWN EXCEPTION during render\n");
    }

    printf("SpriteAnimationDemo: Render cycle completed\n");
}

// Render the current animation frame
void SpriteAnimationDemo::renderCurrentFrame() {
    printf("SpriteAnimationDemo: renderCurrentFrame() - START\n");

    const Animation* anim = animationTable.getAnimation(currentAnimation);
    if (!anim) {
        printf("SpriteAnimationDemo: ERROR - No animation found for '%s'\n", currentAnimation.c_str());
        return;
    }

    if (anim->frames.empty()) {
        printf("SpriteAnimationDemo: ERROR - Animation '%s' has no frames\n", currentAnimation.c_str());
        return;
    }

    printf("SpriteAnimationDemo: Looking for frame %d, direction %d in animation '%s' (%zu total frames)\n",
           currentFrame, static_cast<int>(currentDirection), currentAnimation.c_str(), anim->frames.size());

    // Find the current frame for the current direction
    const AnimationFrame* currentAnimFrame = nullptr;
    for (const auto& frame : anim->frames) {
        if (frame.direction == currentDirection && frame.frameIndex == currentFrame) {
            currentAnimFrame = &frame;
            printf("SpriteAnimationDemo: Found matching frame! frameIndex=%d, direction=%d\n",
                   frame.frameIndex, static_cast<int>(frame.direction));
            break;
        }
    }

    if (!currentAnimFrame) {
        printf("SpriteAnimationDemo: WARNING - No frame found for frameIndex=%d, direction=%d\n",
               currentFrame, static_cast<int>(currentDirection));
        return;
    }

    printf("SpriteAnimationDemo: About to render CF_Sprite at position (%.1f, %.1f)\n", position.x, position.y);

    // Validate the sprite before rendering
    if (currentAnimFrame->sprite.w <= 0 || currentAnimFrame->sprite.h <= 0) {
        printf("SpriteAnimationDemo: ERROR - Invalid sprite dimensions: %dx%d\n",
               currentAnimFrame->sprite.w, currentAnimFrame->sprite.h);
        return;
    }

    printf("SpriteAnimationDemo: Sprite is valid: %dx%d\n", currentAnimFrame->sprite.w, currentAnimFrame->sprite.h);

    // Try to render the actual sprite first
    try {
        printf("SpriteAnimationDemo: Attempting cf_draw_sprite()...\n");
        cf_draw_sprite(&currentAnimFrame->sprite);
        printf("SpriteAnimationDemo: cf_draw_sprite() SUCCESS!\n");
    } catch (...) {
        printf("SpriteAnimationDemo: cf_draw_sprite() FAILED - falling back to colored rectangle\n");

        // Fallback: render colored rectangle if sprite rendering fails
        try {
            printf("SpriteAnimationDemo: Attempting fallback rectangle rendering...\n");

            // Use different colors for different directions for visual feedback
            CF_Color color;
            switch (currentDirection) {
                case Direction::UP:    color = make_color(0.2f, 0.8f, 0.2f, 1.0f); break; // Green
                case Direction::LEFT:  color = make_color(0.8f, 0.2f, 0.2f, 1.0f); break; // Red
                case Direction::DOWN:  color = make_color(0.2f, 0.2f, 0.8f, 1.0f); break; // Blue
                case Direction::RIGHT: color = make_color(0.8f, 0.8f, 0.2f, 1.0f); break; // Yellow
                default:               color = make_color(0.5f, 0.5f, 0.5f, 1.0f); break; // Gray
            }

            printf("SpriteAnimationDemo: Using color for direction %d\n", static_cast<int>(currentDirection));

            cf_draw_push_color(color);
            cf_draw_quad_fill(make_aabb(position, 64, 64), 0.0f);
            cf_draw_pop_color();

            printf("SpriteAnimationDemo: Fallback rectangle rendered successfully\n");
        } catch (...) {
            printf("SpriteAnimationDemo: CRITICAL ERROR - Even fallback rendering failed!\n");
            return;
        }
    }

    printf("SpriteAnimationDemo: Successfully rendered frame %d, direction %d, animation %s at (%.1f, %.1f)\n",
           currentFrame, static_cast<int>(currentDirection), currentAnimation.c_str(),
           position.x, position.y);

    printf("SpriteAnimationDemo: renderCurrentFrame() - END\n");
}

// Render debug information
void SpriteAnimationDemo::renderDebugInfo() {
    printf("SpriteAnimationDemo: renderDebugInfo() - START\n");

    try {
        printf("SpriteAnimationDemo: Setting up debug text rendering...\n");

        // Set text color and position with error checking
        cf_draw_push_color(make_color(1.0f, 1.0f, 1.0f, 1.0f));

        // Render demo info
        v2 textPos = v2(-600, 300);

        printf("SpriteAnimationDemo: Rendering title text...\n");
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

        printf("SpriteAnimationDemo: Rendering state information...\n");

        textPos.y -= 20;
        char stateText[256];
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

        // Animation timing info
        textPos.y -= 30;
        draw_text("Timing Info:", textPos);

        textPos.y -= 20;
        snprintf(stateText, sizeof(stateText), "Frame Timer: %.2fs", frameTimer);
        draw_text(stateText, textPos);

        textPos.y -= 20;
        snprintf(stateText, sizeof(stateText), "Demo Time: %.2fs", demoTime);
        draw_text(stateText, textPos);

        // Animation table info
        textPos.y -= 30;
        draw_text("Animation Info:", textPos);

        textPos.y -= 20;
        snprintf(stateText, sizeof(stateText), "Total Loaded: %zu", animationTable.getAnimationNames().size());
        draw_text(stateText, textPos);

        printf("SpriteAnimationDemo: Rendering animation details...\n");

        int animCount = 0;
        for (const auto& name : animationTable.getAnimationNames()) {
            const Animation* anim = animationTable.getAnimation(name);
            if (anim && animCount < 3) { // Limit to 3 to avoid screen clutter
                textPos.y -= 20;
                snprintf(stateText, sizeof(stateText), "  %s: %zu frames, %.0fms",
                         name.c_str(), anim->frames.size(), anim->totalDuration);
                draw_text(stateText, textPos);
                animCount++;
            }
        }

        // Current frame validation info
        const Animation* currentAnim = animationTable.getAnimation(currentAnimation);
        if (currentAnim) {
            textPos.y -= 30;
            draw_text("Current Frame Debug:", textPos);

            textPos.y -= 20;
            snprintf(stateText, sizeof(stateText), "Looking for frame %d, dir %d", currentFrame, static_cast<int>(currentDirection));
            draw_text(stateText, textPos);

            // Find if current frame exists
            bool frameFound = false;
            for (const auto& frame : currentAnim->frames) {
                if (frame.direction == currentDirection && frame.frameIndex == currentFrame) {
                    frameFound = true;
                    textPos.y -= 20;
                    snprintf(stateText, sizeof(stateText), "Frame found! Sprite: %dx%d", frame.sprite.w, frame.sprite.h);
                    draw_text(stateText, textPos);
                    break;
                }
            }

            if (!frameFound) {
                textPos.y -= 20;
                draw_text("WARNING: Current frame not found!", textPos);
            }
        }

        printf("SpriteAnimationDemo: All debug text rendered successfully\n");
        cf_draw_pop_color();

    } catch (const std::exception& e) {
        printf("SpriteAnimationDemo: EXCEPTION in renderDebugInfo: %s\n", e.what());
        try {
            cf_draw_pop_color(); // Try to restore color state
        } catch (...) {
            printf("SpriteAnimationDemo: Failed to restore color state after exception\n");
        }
    } catch (...) {
        printf("SpriteAnimationDemo: UNKNOWN EXCEPTION in renderDebugInfo\n");
        try {
            cf_draw_pop_color(); // Try to restore color state
        } catch (...) {
            printf("SpriteAnimationDemo: Failed to restore color state after unknown exception\n");
        }
    }

    printf("SpriteAnimationDemo: renderDebugInfo() - END\n");
}

// Check if demo is valid
bool SpriteAnimationDemo::isValid() const {
    return initialized && !animationTable.getAnimationNames().empty();
}
