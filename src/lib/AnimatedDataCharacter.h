#ifndef ANIMATED_DATA_CHARACTER_H
#define ANIMATED_DATA_CHARACTER_H

#include <cute.h>
#include "SpriteAnimationLoader.h"
#include "DataFile.h"

using namespace Cute;

// Forward declaration
class LevelV1;

enum class HitboxShape
{
    SQUARE,
    T_SHAPE,
    L_SHAPE
};

// Demo class to showcase the new SpriteAnimationLoader system
class AnimatedDataCharacter
{
public:
    AnimatedDataCharacter();
    ~AnimatedDataCharacter();

    // Initialize the character with a datafile path
    bool init(const std::string &datafilePath);

    // Update demo state with a move vector
    void update(float dt, v2 moveVector);

    // Render the demo
    void render();

    // Render the demo at a specific position
    void render(v2 renderPosition);

    // deprecated
    void handleInput();

    // Check if demo is valid
    bool isValid() const;

    // Get current position
    v2 getPosition() const;

    // Set current position
    void setPosition(v2 newPosition);

    // Set the level for agent queries
    void setLevel(LevelV1* level);

    // Get hitbox in world coordinates
    CF_Aabb getHitbox() const;

    // Hitbox shape methods
    void setShape(HitboxShape shape);

private:
    // The animation loader
    SpriteAnimationLoader loader;

    // DataFile containing character configuration
    DataFile datafile;

    // Animation table containing all skeleton animations
    AnimationTable animationTable;

    // Current animation state
    std::string currentAnimation;
    Direction currentDirection;
    int currentFrame;
    float frameTimer;

    // Demo state
    bool initialized;
    float demoTime;

    // Input state
    bool keysPressed[4];   // UP, LEFT, DOWN, RIGHT
    bool animationKeys[2]; // 1 for idle, 2 for walkcycle

    // Demo parameters
    float directionChangeTime;
    float animationChangeTime;

    // Position for rendering
    v2 position;

    // Movement tracking for animation switching
    bool wasMoving;

    // Hitbox state
    bool hitboxActive;
    float hitboxSize;
    float hitboxDistance;
    HitboxShape hitboxShape;
    LevelV1* level; // Pointer to level for agent queries

    // Helper methods
    void cycleDirection();
    void cycleAnimation();
    void updateAnimation(float dt);
    void renderCurrentFrame();
    void renderCurrentFrameAt(v2 renderPosition);
    void renderDebugInfo();
    void renderHitbox();


    std::vector<CF_Aabb> getHitboxTShape() const;
    std::vector<CF_Aabb> getHitboxLShape() const;
};

#endif // ANIMATED_DATA_CHARACTER_H
