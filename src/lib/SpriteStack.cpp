#include "SpriteBatch.h"
#include <cute.h>

// Default constructor
SpriteBatch::SpriteBatch()
    : Sprite()
    , currentDirection(Direction::DOWN)
    , frameCount(1)
    , frameSize(v2(64, 64))
    , currentFrame(0)
    , renderScale(v2(1.0f, 1.0f))
{
}

// Constructor with texture path
SpriteBatch::SpriteBatch(const char* texture_path)
    : Sprite(texture_path)
    , currentDirection(Direction::DOWN)
    , frameCount(1)
    , frameSize(v2(64, 64))
    , currentFrame(0)
    , renderScale(v2(1.0f, 1.0f))
{
    // Try to auto-detect frame size and count from texture
    // For now, use defaults - this can be enhanced later
}

// Constructor with texture path and frame specifications
SpriteBatch::SpriteBatch(const char* texture_path, int frames, v2 frameDimensions)
    : Sprite(texture_path)
    , currentDirection(Direction::DOWN)
    , frameCount(frames)
    , frameSize(frameDimensions)
    , currentFrame(0)
    , renderScale(v2(1.0f, 1.0f))
{
}

// Constructor with texture path, frame specifications, and render scale
SpriteBatch::SpriteBatch(const char* texture_path, int frames, v2 frameDimensions, v2 renderScale)
    : Sprite(texture_path)
    , currentDirection(Direction::DOWN)
    , frameCount(frames)
    , frameSize(frameDimensions)
    , currentFrame(0)
    , renderScale(renderScale)
{
}

// Destructor
SpriteBatch::~SpriteBatch() {
    // Base Sprite destructor handles cleanup
}

// Direction management
void SpriteBatch::setDirection(Direction direction) {
    currentDirection = direction;
}

Direction SpriteBatch::getDirection() const {
    return currentDirection;
}

// Frame management
void SpriteBatch::setFrame(int frameIndex) {
    if (frameIndex >= 0 && frameIndex < frameCount) {
        currentFrame = frameIndex;
    }
}

int SpriteBatch::getFrame() const {
    return currentFrame;
}

int SpriteBatch::getFrameCount() const {
    return frameCount;
}

// Frame size and dimensions
void SpriteBatch::setFrameSize(v2 size) {
    frameSize = size;
}

v2 SpriteBatch::getFrameSize() const {
    return frameSize;
}

// Render scale
void SpriteBatch::setRenderScale(v2 scale) {
    renderScale = scale;
}

v2 SpriteBatch::getRenderScale() const {
    return renderScale;
}

// Utility methods
void SpriteBatch::nextFrame() {
    currentFrame = (currentFrame + 1) % frameCount;
}

void SpriteBatch::previousFrame() {
    currentFrame = (currentFrame - 1 + frameCount) % frameCount;
}

void SpriteBatch::resetFrame() {
    currentFrame = 0;
}

// Internal methods
v2 SpriteBatch::calculateFrameUV(int frameIndex, Direction direction) const {
    // Calculate UV coordinates for the given frame and direction
    // For a stacked sprite sheet:
    // - Each row represents a direction (UP=0, LEFT=1, DOWN=2, RIGHT=3)
    // - Since we only have 1 frame per direction, frameIndex should always be 0
    // - The direction determines which row (Y coordinate)

    // Calculate the UV rectangle for the current frame
    // This represents the top-left corner of the frame in UV space
    float u = 0.0f; // Single frame, so always at left edge
    float v = (float)static_cast<int>(direction) / 4.0f; // 4 directions, so each takes 1/4 of height

    return v2(u, v);
}

void SpriteBatch::render() {
    if (!isValid()) return;

    // Calculate UV coordinates for current frame and direction
    v2 frameUV = calculateFrameUV(currentFrame, currentDirection);

    // Save current draw state
    cf_draw_push();

    // Apply transforms using the correct Cute Framework API
    cf_draw_translate_v2(getPosition());

    // Apply render scale to make the frame the right size
    v2 finalScale = getScale() * renderScale;
    cf_draw_scale_v2(finalScale);

    cf_draw_rotate(getRotation());

    // For now, use the base Sprite render
    // TODO: Implement proper UV coordinate rendering to show only the specific frame
    Sprite::render();

    // Restore draw state
    cf_draw_pop();
}

void SpriteBatch::update(float dt) {
    // Update sprite animation if needed
    cf_sprite_update(&sprite);
}
