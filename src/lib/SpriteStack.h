#pragma once

#include <cute.h>
#include <string>
#include "Sprite.h"

using namespace Cute;

// Direction enum for sprite batches
enum class Direction {
    UP = 0,
    LEFT = 1,
    DOWN = 2,
    RIGHT = 3
};

// Enhanced sprite class with directional support for stacked animation frames
class SpriteBatch : public Sprite {
private:
    Direction currentDirection;    // Current direction being displayed
    int frameCount;               // Number of frames per direction
    v2 frameSize;                 // Size of each individual frame
    int currentFrame;             // Current frame index (0 to frameCount-1)
    v2 renderScale;               // Scale factor for rendering (to make frames smaller)

    // Internal methods
    void updateUVCoordinates();

public:
    // Constructors
    SpriteBatch();
    SpriteBatch(const char* texture_path);
    SpriteBatch(const char* texture_path, int frames, v2 frameDimensions);
    SpriteBatch(const char* texture_path, int frames, v2 frameDimensions, v2 renderScale);
    virtual ~SpriteBatch();

    // Direction management
    void setDirection(Direction direction);
    Direction getDirection() const;

    // Frame management
    void setFrame(int frameIndex);
    int getFrame() const;
    int getFrameCount() const;

    // Frame size and dimensions
    void setFrameSize(v2 size);
    v2 getFrameSize() const;

    // Render scale
    void setRenderScale(v2 scale);
    v2 getRenderScale() const;

    // Utility methods
    void nextFrame();
    void previousFrame();
    void resetFrame();

    // UV coordinate calculation (public for testing)
    v2 calculateFrameUV(int frameIndex, Direction direction) const;

    // Override base Sprite methods
    void render() override;
    void update(float dt) override;
};
