#include "SpriteBatchDemo.h"
#include <cute.h>
#include <sstream>
#include <iomanip>

SpriteBatchDemo::SpriteBatchDemo()
    : demoTime(0.0f)
    , currentDirection(Direction::DOWN)
    , currentFrame(0)
{
}

SpriteBatchDemo::~SpriteBatchDemo() {
    // Cleanup handled automatically
}

bool SpriteBatchDemo::initialize() {
    printf("Initializing SpriteBatchDemo...\n");

    // Create sprite batches for skeleton parts
    // Idle assets: 4 frames stacked vertically (UP, LEFT, DOWN, RIGHT)
    // Each frame: 64x64 pixels, total sheet: 64x256
    // Render scale: 2.0 to make them visible at reasonable size
    printf("Creating skeleton body sprite batch...\n");
    skeleton_body = SpriteBatch("assets/Art/AnimationsSheets/idle/BODY_skeleton.png", 4, v2(64, 64), v2(2.0f, 2.0f));

    printf("Creating skeleton head sprite batch...\n");
    skeleton_head = SpriteBatch("assets/Art/AnimationsSheets/idle/HEAD_chain_armor_helmet.png", 4, v2(64, 64), v2(2.0f, 2.0f));

    // Position sprites at center of screen
    skeleton_body.setPosition(v2(0, 0));
    skeleton_head.setPosition(v2(0, 0));

    // Set initial direction
    skeleton_body.setDirection(currentDirection);
    skeleton_head.setDirection(currentDirection);

    printf("SpriteBatchDemo initialization complete\n");
    return true;
}

void SpriteBatchDemo::handleInput() {
    // Handle arrow key input for direction changes
    if (key_just_pressed(CF_KEY_UP)) {
        currentDirection = Direction::UP;
        skeleton_body.setDirection(currentDirection);
        skeleton_head.setDirection(currentDirection);
    }
    if (key_just_pressed(CF_KEY_LEFT)) {
        currentDirection = Direction::LEFT;
        skeleton_body.setDirection(currentDirection);
        skeleton_head.setDirection(currentDirection);
    }
    if (key_just_pressed(CF_KEY_DOWN)) {
        currentDirection = Direction::DOWN;
        skeleton_body.setDirection(currentDirection);
        skeleton_head.setDirection(currentDirection);
    }
    if (key_just_pressed(CF_KEY_RIGHT)) {
        currentDirection = Direction::RIGHT;
        skeleton_body.setDirection(currentDirection);
        skeleton_head.setDirection(currentDirection);
    }

    // Handle frame changes with A/D keys
    if (key_just_pressed(CF_KEY_A)) {
        currentFrame = (currentFrame - 1 + skeleton_body.getFrameCount()) % skeleton_body.getFrameCount();
        skeleton_body.setFrame(currentFrame);
        skeleton_head.setFrame(currentFrame);
    }
    if (key_just_pressed(CF_KEY_D)) {
        currentFrame = (currentFrame + 1) % skeleton_body.getFrameCount();
        skeleton_body.setFrame(currentFrame);
        skeleton_head.setFrame(currentFrame);
    }

    // Reset demo with R key
    if (key_just_pressed(CF_KEY_R)) {
        reset();
    }
}

void SpriteBatchDemo::updateDemo(float dt) {
    demoTime += dt;
}

void SpriteBatchDemo::renderSprites() {
    // Render the skeleton sprites
    skeleton_body.render();
    skeleton_head.render();
}

void SpriteBatchDemo::update(float dt) {
    handleInput();
    updateDemo(dt);

    // Update sprite animations
    skeleton_body.update(dt);
    skeleton_head.update(dt);
}

void SpriteBatchDemo::render() {
    renderSprites();
    renderDemoInfo();
}

void SpriteBatchDemo::reset() {
    demoTime = 0.0f;
    currentDirection = Direction::DOWN;
    currentFrame = 0;

    skeleton_body.setDirection(currentDirection);
    skeleton_head.setDirection(currentDirection);
    skeleton_body.setFrame(currentFrame);
    skeleton_head.setFrame(currentFrame);
}

void SpriteBatchDemo::renderDemoInfo() {
    // Draw demo information in the upper-left quadrant
    v2 text_position = v2(-300, 200);
    draw_text("SpriteBatch Demo - Directional Sprites", text_position);

    // Convert numbers to strings for display
    std::stringstream ss;

    ss.str("");
    ss << "Current Direction: ";
    switch (currentDirection) {
        case Direction::UP: ss << "UP"; break;
        case Direction::LEFT: ss << "LEFT"; break;
        case Direction::DOWN: ss << "DOWN"; break;
        case Direction::RIGHT: ss << "RIGHT"; break;
    }
    draw_text(ss.str().c_str(), v2(-300, 180));

    ss.str("");
    ss << "Current Frame: " << currentFrame << "/" << (skeleton_body.getFrameCount() - 1);
    draw_text(ss.str().c_str(), v2(-300, 160));

    ss.str("");
    ss << "Frame Size: " << skeleton_body.getFrameSize().x << "x" << skeleton_body.getFrameSize().y;
    draw_text(ss.str().c_str(), v2(-300, 140));

    ss.str("");
    ss << "Render Scale: 2.0x (128x128 on screen)";
    draw_text(ss.str().c_str(), v2(-300, 120));

    ss.str("");
    ss << "Layout: 4 frames x 4 directions (64x256 total)";
    draw_text(ss.str().c_str(), v2(-300, 100));

    ss.str("");
    ss << "Demo Time: " << std::fixed << std::setprecision(1) << demoTime << "s";
    draw_text(ss.str().c_str(), v2(-300, 80));

    // Debug info for UV coordinates
    ss.str("");
    v2 uv = skeleton_body.calculateFrameUV(currentFrame, currentDirection);
    ss << "UV Coordinates: (" << std::fixed << std::setprecision(2) << uv.x << ", " << uv.y << ")";
    draw_text(ss.str().c_str(), v2(-300, 60));

    // Controls
    draw_text("Controls:", v2(-300, 30));
    draw_text("Arrow Keys - Change Direction", v2(-300, 10));
    draw_text("A/D - Previous/Next Frame", v2(-300, -10));
    draw_text("R - Reset Demo", v2(-300, -30));

    // Asset information
    draw_text("Assets:", v2(-300, -60));
    draw_text("Body: assets/Art/AnimationsSheets/idle/BODY_skeleton.png", v2(-300, -80));
    draw_text("Head: assets/Art/AnimationsSheets/idle/HEAD_chain_armor_helmet.png", v2(-300, -100));
}
