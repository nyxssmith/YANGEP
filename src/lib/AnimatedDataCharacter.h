#ifndef ANIMATED_DATA_CHARACTER_H
#define ANIMATED_DATA_CHARACTER_H

#include <cute.h>
#include "SpriteAnimationLoader.h"
#include "DataFile.h"

using namespace Cute;

// Demo class to showcase the new SpriteAnimationLoader system
class AnimatedDataCharacter
{
public:
    AnimatedDataCharacter();
    ~AnimatedDataCharacter();

    // Initialize the character with a datafile path
    bool init(const std::string &datafilePath);

    // Update demo state
    void update(float dt);

    // Render the demo
    void render();

    // Render the demo at a specific position
    void render(v2 renderPosition);

    // Handle input for demo controls
    void handleInput();

    // Check if demo is valid
    bool isValid() const;

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

    // Helper methods
    void cycleDirection();
    void cycleAnimation();
    void updateAnimation(float dt);
    void renderCurrentFrame();
    void renderCurrentFrameAt(v2 renderPosition);
    void renderDebugInfo();
};

#endif // ANIMATED_DATA_CHARACTER_H
