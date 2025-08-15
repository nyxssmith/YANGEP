#include "Sprite.h"
#include <cute.h>

// Default constructor
Sprite::Sprite()
    : position(v2(0, 0))
    , scale(v2(1, 1))
    , rotation(0.0f)
    , visible(true)
{
    // Initialize with empty sprite
    sprite = cf_sprite_defaults();
}

// Constructor with texture path
Sprite::Sprite(const char* texture_path)
    : Sprite() // Call default constructor
{
    // Load sprite from PNG using Cute Framework's easy sprite system
    CF_Result result;
    sprite = cf_make_easy_sprite_from_png(texture_path, &result);

    if (Cute::is_error(result)) {
        printf("Failed to load sprite: %s\n", texture_path);
        sprite = cf_sprite_defaults();
    }
}

// Destructor
Sprite::~Sprite() {
    // Cute Framework handles sprite cleanup automatically
}

// Core methods
void Sprite::render() {
    if (!visible || !isValid()) return;

    // Save current draw state
    cf_draw_push();

    // Apply transforms using the correct Cute Framework API
    cf_draw_translate_v2(position);
    cf_draw_scale_v2(scale);
    cf_draw_rotate(rotation);

    // Draw the sprite
    cf_draw_sprite(&sprite);

    // Restore draw state
    cf_draw_pop();
}

void Sprite::update(float dt) {
    // Update sprite animation if needed
    cf_sprite_update(&sprite);
}

// Transform methods
void Sprite::setPosition(v2 pos) {
    position = pos;
}

void Sprite::setScale(v2 scale) {
    this->scale = scale;
}

void Sprite::setRotation(float rotation) {
    this->rotation = rotation;
}

void Sprite::setTransform(v2 pos, float rot, v2 scale) {
    position = pos;
    rotation = rot;
    this->scale = scale;
}

// Utility transforms
void Sprite::translate(v2 offset) {
    position = position + offset;
}

void Sprite::rotate(float angle) {
    rotation += angle;
}

void Sprite::scaleBy(v2 factor) {
    scale = scale * factor;
}

// Getters
v2 Sprite::getPosition() const {
    return position;
}

v2 Sprite::getScale() const {
    return scale;
}

float Sprite::getRotation() const {
    return rotation;
}

bool Sprite::isVisible() const {
    return visible;
}

// Setters
void Sprite::setVisible(bool visible) {
    this->visible = visible;
}

// Utility
bool Sprite::isValid() const {
    return sprite.name != nullptr;
}
