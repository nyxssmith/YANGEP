#pragma once

#include <cute.h>
#include <string>
#include "AnimatedSprite.h"

using namespace Cute;

class AnimationDemo {
private:
    // Animated sprites for different states
    AnimatedSprite skeleton_body_idle;
    AnimatedSprite skeleton_head_idle;
    AnimatedSprite skeleton_body_walk;
    AnimatedSprite skeleton_head_walk;

    // Current active sprites
    AnimatedSprite* current_body;
    AnimatedSprite* current_head;

        // Demo state
    float demoTime;
    bool showIdle;
    bool showWalkCycle;
    SkeletonDirection currentDirection;

    // Animation controls
    float animationSpeed;
    bool animationPaused;

    // Demo methods
    void setupAnimations();
    void handleInput();
    void updateAnimations(float dt);
    void renderSprites();
    void renderDemoInfo();
    void updateAnimationSpeeds();
    void switchToAnimation(bool isIdle);

public:
    AnimationDemo();
    ~AnimationDemo();

    bool initialize();
    void update(float dt);
    void render();
    void reset();
};
