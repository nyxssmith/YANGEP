#include "PNGSpriteDemo.h"
#include <cute.h>
#include <cute_draw.h>
#include <cute_math.h>
#include <cstdlib>

using namespace Cute;

// Constructor
PNGSpriteDemo::PNGSpriteDemo()
    : sprite(nullptr), timeSinceStart(0.0f), directionChangeTimer(0.0f), animationChangeTimer(0.0f),
      currentDirectionIndex(0), currentAnimationIndex(0)
{
}

// Destructor
PNGSpriteDemo::~PNGSpriteDemo() {
    if (sprite) {
        delete sprite;
        sprite = nullptr;
    }
}

// Initialize the demo
bool PNGSpriteDemo::initialize() {
    // Create sprite
    sprite = new PNGSprite();

    // Load animations (this will exit(1) on failure)
    sprite->loadAnimations("assets/Art/AnimationsSheets/idle/BODY_skeleton.png",
                          "assets/Art/AnimationsSheets/walkcycle/BODY_skeleton.png");

    // Verify sprite is valid
    if (!sprite->isValid()) {
        printf("FATAL ERROR: Sprite validation failed after loading animations\n");
        exit(1);
    }

    // Set initial state
    sprite->setDirection(Direction::DOWN);
    sprite->setAnimation("idle");

    // Initialize timers
    timeSinceStart = 0.0f;
    directionChangeTimer = 0.0f;
    animationChangeTimer = 0.0f;

    printf("PNG Sprite Demo initialized successfully\n");
    printf("Auto-cycling demo: directions every %.1fs, animations every %.1fs\n",
           DIRECTION_CHANGE_INTERVAL, ANIMATION_CHANGE_INTERVAL);

    return true;
}

// Update demo state
void PNGSpriteDemo::update(float dt) {
    if (!sprite) return;

    // Update timers
    timeSinceStart += dt;
    directionChangeTimer += dt;
    animationChangeTimer += dt;

    // Update demo state
    updateDemoState(dt);

    // Update sprite
    sprite->update(dt);
}

// Update demo state (auto-cycling)
void PNGSpriteDemo::updateDemoState(float dt) {
    // Cycle direction
    if (directionChangeTimer >= DIRECTION_CHANGE_INTERVAL) {
        cycleDirection();
        directionChangeTimer = 0.0f;
    }

    // Cycle animation
    if (animationChangeTimer >= ANIMATION_CHANGE_INTERVAL) {
        cycleAnimation();
        animationChangeTimer = 0.0f;
    }
}

// Cycle through directions
void PNGSpriteDemo::cycleDirection() {
    currentDirectionIndex = (currentDirectionIndex + 1) % 4;
    Direction newDirection = static_cast<Direction>(currentDirectionIndex);

    sprite->setDirection(newDirection);

    printf("AUTO: Direction changed to: %d (%s)\n",
           currentDirectionIndex, getDirectionName(newDirection));
}

// Cycle through animations
void PNGSpriteDemo::cycleAnimation() {
    currentAnimationIndex = (currentDirectionIndex + 1) % 2;
    const char* newAnimation = getAnimationName(currentAnimationIndex);

    sprite->setAnimation(newAnimation);

    printf("AUTO: Animation changed to: %s\n", newAnimation);
}

// Get direction name
const char* PNGSpriteDemo::getDirectionName(Direction direction) const {
    switch (direction) {
        case Direction::UP: return "up";
        case Direction::LEFT: return "left";
        case Direction::DOWN: return "down";
        case Direction::RIGHT: return "right";
        default: return "unknown";
    }
}

// Get animation name
const char* PNGSpriteDemo::getAnimationName(int index) const {
    switch (index) {
        case 0: return "idle";
        case 1: return "walkcycle";
        default: return "unknown";
    }
}

// Render the demo
void PNGSpriteDemo::render() {
    if (!sprite) return;

    // Render sprite
    sprite->render();
}

// Render demo information
void PNGSpriteDemo::renderDemoInfo() {
    if (!sprite) return;

    // Demo title
    cf_draw_text("PNG Sprite Demo - Auto-Cycling", v2(-400, 300), -1);

    // Current state
    cf_draw_text("Current Direction:", v2(-400, 250), -1);
    cf_draw_text(getDirectionName(sprite->getDirection()), v2(-200, 250), -1);

    cf_draw_text("Current Animation:", v2(-400, 220), -1);
    cf_draw_text(sprite->getCurrentAnimation(), v2(-200, 220), -1);

    // Timing info
    cf_draw_text("Time Running:", v2(-400, 190), -1);
    char timeStr[32];
    snprintf(timeStr, sizeof(timeStr), "%.1fs", timeSinceStart);
    cf_draw_text(timeStr, v2(-200, 190), -1);

    // Next changes
    cf_draw_text("Next Direction Change:", v2(-400, 160), -1);
    snprintf(timeStr, sizeof(timeStr), "%.1fs", DIRECTION_CHANGE_INTERVAL - directionChangeTimer);
    cf_draw_text(timeStr, v2(-200, 160), -1);

    cf_draw_text("Next Animation Change:", v2(-400, 130), -1);
    snprintf(timeStr, sizeof(timeStr), "%.1fs", ANIMATION_CHANGE_INTERVAL - animationChangeTimer);
    cf_draw_text(timeStr, v2(-200, 130), -1);

    // Instructions
    cf_draw_text("Demo automatically cycles through directions and animations", v2(-400, 70), -1);
    cf_draw_text("No input required - just watch the debug output!", v2(-400, 40), -1);

    // Debug info
    cf_draw_text("DEBUG: Check console for detailed state changes", v2(-400, -50), -1);
    cf_draw_text("DEBUG: Sprite dimensions:", v2(-400, -80), -1);
    snprintf(timeStr, sizeof(timeStr), "%dx%d", sprite->getWidth(), sprite->getHeight());
    cf_draw_text(timeStr, v2(-200, -80), -1);
}
