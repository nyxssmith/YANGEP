#ifndef PNG_SPRITE_H
#define PNG_SPRITE_H

#include <cute.h>
#include <cute_png_cache.h>
#include "SpriteSheetSplitter.h"

using namespace Cute;

// PNG-based sprite class using Cute Framework's PNG API
class PNGSprite {
public:
    PNGSprite();
    ~PNGSprite();

    // Core methods
    bool loadAnimations(const char* idle_path, const char* walkcycle_path);
    void render();
    void update(float dt);

    // Direction management
    void setDirection(Direction direction);
    Direction getDirection() const;

    // Animation management
    void setAnimation(const char* animation_name);
    const char* getCurrentAnimation() const;

    // Transform methods
    void setPosition(v2 pos);
    void setScale(v2 scale);
    void setRotation(float rotation);
    v2 getPosition() const;
    v2 getScale() const;
    float getRotation() const;

    // Utility
    bool isValid() const;
    int getWidth() const;
    int getHeight() const;

private:
    // Sprite sheet splitters
    SpriteSheetSplitter idleSplitter;      // Handles 64x256 idle sheet
    SpriteSheetSplitter walkcycleSplitter; // Handles 576x256 walkcycle sheet

    // Animation tables (now using virtual PNGs)
    const htbl CF_Animation** idleAnimationTable;
    const htbl CF_Animation** walkcycleAnimationTable;

    // Sprites
    CF_Sprite idleSprite;
    CF_Sprite walkcycleSprite;

    // Current state
    const char* currentAnimation;  // "idle" or "walkcycle"
    Direction currentDirection;    // UP, LEFT, DOWN, RIGHT

    // Transform data
    v2 position;
    v2 scale;
    float rotation;

    // Private methods
    bool createIdleAnimationTable();
    bool createWalkcycleAnimationTable();
    void cleanup();
};

#endif // PNG_SPRITE_H
