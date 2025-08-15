#pragma once

#include <cute.h>
#include "Sprite.h"
#include "DataFile.h"

class SpriteDemo {
private:
    // Demo sprites
    Sprite body_sprite;
    Sprite head_sprite;
    Sprite demo_sprite1;
    Sprite demo_sprite2;

    // Demo state
    float rotation;
    float scale;
    bool growing;
    v2 body_position;

    // Configuration
    DataFile skeleton_config;

    // Demo methods
    void setupSprites();
    void handleInput();
    void updateAnimation(float dt);
    void renderSprites();
    void renderDemoInfo();

public:
    SpriteDemo();
    ~SpriteDemo();

    bool initialize();
    void update(float dt);
    void render();
    void reset();
};
