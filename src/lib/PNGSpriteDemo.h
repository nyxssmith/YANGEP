#ifndef PNG_SPRITE_DEMO_H
#define PNG_SPRITE_DEMO_H

#include <cute.h>
#include "PNGSprite.h"

using namespace Cute;

// Demo class for PNG-based sprites
class PNGSpriteDemo {
public:
    PNGSpriteDemo();
    ~PNGSpriteDemo();

    // Core methods
    bool initialize();
    void update(float dt);
    void render();
    void renderDemoInfo();

private:
    // Demo state
    PNGSprite* sprite;
    float timeSinceStart;
    float directionChangeTimer;
    float animationChangeTimer;

    // Auto-cycling settings
    const float DIRECTION_CHANGE_INTERVAL = 2.0f;  // Change direction every 2 seconds
    const float ANIMATION_CHANGE_INTERVAL = 4.0f;  // Change animation every 4 seconds

    // Current demo state
    int currentDirectionIndex;
    int currentAnimationIndex;

    // Demo info
    void updateDemoState(float dt);
    void cycleDirection();
    void cycleAnimation();
    const char* getDirectionName(Direction direction) const;
    const char* getAnimationName(int index) const;
};

#endif // PNG_SPRITE_DEMO_H
