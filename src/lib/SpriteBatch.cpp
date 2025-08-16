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
    printf("DEBUG: setDirection called with direction=%d, currentDirection was=%d\n",
           static_cast<int>(direction), static_cast<int>(currentDirection));
    currentDirection = direction;
    printf("DEBUG: currentDirection is now=%d\n", static_cast<int>(currentDirection));
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

    // Calculate the UV coordinates for the current frame
    // This represents the top-left corner of the frame in UV space
    float u = 0.0f; // Single frame, so always at left edge
    float v = (float)static_cast<int>(direction) / 4.0f; // 4 directions, so each takes 1/4 of height

    // Debug: Let's see what we're calculating
    printf("DEBUG: calculateFrameUV: frame=%d, direction=%d, UV=(%.2f, %.2f)\n",
           frameIndex, static_cast<int>(direction), u, v);

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

    // Get the sprite's texture dimensions
    int texWidth = getTextureWidth();
    int texHeight = getTextureHeight();

    if (texWidth > 0 && texHeight > 0) {
        // Calculate the UV rectangle for the current frame
        float frameWidth = frameSize.x / (float)texWidth;
        float frameHeight = frameSize.y / (float)texHeight;

        // Create UV rectangle for the current frame
        float minx = frameUV.x;
        float miny = frameUV.y;
        float maxx = frameUV.x + frameWidth;
        float maxy = frameUV.y + frameHeight;

        // Instead of calling Sprite::render(), we need to render our own quad
        // with the correct UV coordinates. However, we need access to the sprite's texture.

        // For now, let's use a workaround: render a colored quad to show the frame bounds
        // This will help us visualize what we're trying to achieve

        // Set a distinct color for debugging
        cf_draw_push_color(make_color(1.0f, 0.0f, 0.0f, 0.5f)); // Semi-transparent red
        
        // Render a quad that represents our frame
        // We'll use the frame dimensions and position
        v2 framePos = v2(-frameSize.x * 0.5f, -frameSize.y * 0.5f); // Center the frame
        CF_Aabb frameAABB = make_aabb(framePos, frameSize.x, frameSize.y);
        cf_draw_quad_fill(frameAABB, 0.0f);
        
        cf_draw_pop_color();

        // Debug output to show what UV coordinates we're calculating
        printf("DEBUG: Rendering frame %d, direction %d, UV: (%.2f, %.2f) to (%.2f, %.2f)\n",
               currentFrame, static_cast<int>(currentDirection), minx, miny, maxx, maxy);
        printf("DEBUG: Frame size: %.0fx%.0f, Texture size: %dx%d\n",
               frameSize.x, frameSize.y, texWidth, texHeight);

        // TODO: Implement proper UV coordinate rendering using one of these approaches:
        // 1. Use the spritebatch system directly with custom UV coordinates
        // 2. Create a custom shader that supports UV coordinate clipping
        // 3. Use the low-level graphics API to render a textured quad with UV coordinates

        // The challenge is that we need access to the sprite's texture handle
        // to implement proper UV rendering. The current approach shows the frame bounds
        // but doesn't render the actual texture content.

    } else {
        // Fallback if texture dimensions are not available
        printf("ERROR: Cannot render SpriteBatch - texture dimensions unavailable\n");
    }

    // Restore draw state
    cf_draw_pop();
}

void SpriteBatch::update(float dt) {
    // Update sprite animation if needed
    Sprite::update(dt);
}
