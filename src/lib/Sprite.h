#pragma once

#include <cute.h>
#include <string>

using namespace Cute;

class Sprite {
private:
    CF_Sprite sprite;      // Cute Framework sprite handle
    v2 position;           // Position in world space
    v2 scale;              // Scale factors (x, y)
    float rotation;         // Rotation in radians
    bool visible;          // Visibility flag

public:
    // Constructors
    Sprite();
    Sprite(const char* texture_path);
    ~Sprite();

    // Core methods
    void render();
    void update(float dt);

    // Transform methods
    void setPosition(v2 pos);
    void setScale(v2 scale);
    void setRotation(float rotation);
    void setTransform(v2 pos, float rot, v2 scale);

    // Utility transforms
    void translate(v2 offset);
    void rotate(float angle);
    void scaleBy(v2 factor);

    // Getters
    v2 getPosition() const;
    v2 getScale() const;
    float getRotation() const;
    bool isVisible() const;

    // Setters
    void setVisible(bool visible);

    // Utility
    bool isValid() const;
};
