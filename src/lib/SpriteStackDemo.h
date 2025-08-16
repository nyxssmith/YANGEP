#pragma once

#include <cute.h>
#include <string>
#include "SpriteBatch.h"

using namespace Cute;

class SpriteBatchDemo {
private:
    // Sprite batches for different character parts
    SpriteBatch skeleton_body;
    SpriteBatch skeleton_head;

    // Demo state
    float demoTime;
    Direction currentDirection;
    int currentFrame;

    // Demo methods
    void handleInput();
    void updateDemo(float dt);
    void renderSprites();
    void renderDemoInfo();

public:
    SpriteBatchDemo();
    ~SpriteBatchDemo();

    bool initialize();
    void update(float dt);
    void render();
    void reset();
};
