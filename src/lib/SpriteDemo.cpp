#include "SpriteDemo.h"
#include <cute.h>
#include <cmath>
#include <sstream>
#include <iomanip> // Required for std::fixed and std::setprecision

SpriteDemo::SpriteDemo()
    : rotation(0.0f)
    , scale(1.0f)
    , growing(true)
    , body_position(v2(320, 240))
{
    // Initialize configuration
    skeleton_config = DataFile();
}

SpriteDemo::~SpriteDemo() {
    // Cleanup handled automatically
}

bool SpriteDemo::initialize() {
        // Load skeleton configuration
    if (!skeleton_config.load("tests/assets/skeleton/skeleton.json")) {
        printf("Failed to load skeleton.json\n");
        return false;
    }

    // Create sprites from skeleton assets
    body_sprite = Sprite("tests/assets/skeleton/BODY_skeleton.png");
    head_sprite = Sprite("tests/assets/skeleton/HEAD_chain_armor_helmet.png");

    // Create additional demo sprites
    demo_sprite1 = Sprite("tests/assets/skeleton/BODY_skeleton.png");
    demo_sprite2 = Sprite("tests/assets/skeleton/HEAD_chain_armor_helmet.png");

    // Position sprites for demo
    body_sprite.setPosition(body_position); // Center of screen
    head_sprite.setPosition(v2(body_position.x, body_position.y - 40)); // Above body

    // Position demo sprites
    demo_sprite1.setPosition(v2(100, 100));
    demo_sprite2.setPosition(v2(540, 100));

    // Set different scales and rotations for variety
    demo_sprite1.setScale(v2(0.5f, 0.5f));
    demo_sprite2.setScale(v2(1.5f, 1.5f));

    return true;
}

void SpriteDemo::handleInput() {
    // Handle keyboard input
    if (cf_key_just_pressed(CF_KEY_SPACE)) {
        reset();
    }

    // Arrow key movement
    if (cf_key_down(CF_KEY_LEFT)) body_position.x -= 2.0f;
    if (cf_key_down(CF_KEY_RIGHT)) body_position.x += 2.0f;
    if (cf_key_down(CF_KEY_UP)) body_position.y -= 2.0f;
    if (cf_key_down(CF_KEY_DOWN)) body_position.y += 2.0f;

    // Update sprite positions
    body_sprite.setPosition(body_position);
    head_sprite.setPosition(v2(body_position.x, body_position.y - 40)); // Keep head above body
}

void SpriteDemo::updateAnimation(float dt) {
    // Demo animation
    rotation += 0.02f; // Slow rotation

    // Pulsing scale effect
    if (growing) {
        scale += 0.01f;
        if (scale >= 1.2f) growing = false;
    } else {
        scale -= 0.01f;
        if (scale <= 0.8f) growing = true;
    }

    // Apply transforms to sprites
    body_sprite.setRotation(rotation);
    body_sprite.setScale(v2(scale, scale));
    head_sprite.setRotation(rotation * 0.5f); // Head rotates slower
    head_sprite.setScale(v2(scale * 0.8f, scale * 0.8f)); // Head slightly smaller

    // Animate demo sprites
    demo_sprite1.setRotation(rotation * -0.8f); // Counter-rotation
    demo_sprite2.setRotation(rotation * 1.2f); // Faster rotation

    // Bouncing effect for demo sprites
    float bounce = sin(rotation * 2.0f) * 20.0f;
    demo_sprite1.setPosition(v2(100, 100 + bounce));
    demo_sprite2.setPosition(v2(540, 100 - bounce));
}

void SpriteDemo::renderSprites() {
    // Render sprites
    body_sprite.render();
    head_sprite.render();
    demo_sprite1.render();
    demo_sprite2.render();
}

void SpriteDemo::renderDemoInfo() {
    // Draw demo info text
    v2 text_position = v2(10, 10);
    draw_text("Sprite Demo - Skeleton Character", text_position);

    // Convert numbers to strings for display
    std::stringstream ss;

    ss.str("");
    ss << "Rotation: " << (int)(rotation * 180.0f / 3.14159f) << "°";
    draw_text(ss.str().c_str(), v2(10, 30));

    ss.str("");
    ss << "Scale: " << std::fixed << std::setprecision(2) << scale;
    draw_text(ss.str().c_str(), v2(10, 50));

    ss.str("");
    ss << "Position: (" << (int)body_position.x << ", " << (int)body_position.y << ")";
    draw_text(ss.str().c_str(), v2(10, 70));

    draw_text("Press SPACE to reset, ARROWS to move", v2(10, 90));
    draw_text("Sprite System Phase 1.1 Demo", v2(10, 110));
}

void SpriteDemo::update(float dt) {
    handleInput();
    updateAnimation(dt);
}

void SpriteDemo::render() {
    renderSprites();
    renderDemoInfo();
}

void SpriteDemo::reset() {
    rotation = 0.0f;
    scale = 1.0f;
    body_position = v2(320, 240);
    body_sprite.setPosition(body_position);
    head_sprite.setPosition(v2(body_position.x, body_position.y - 40));
}
