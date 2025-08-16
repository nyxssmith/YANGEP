#pragma once

#include <cute.h>
#include <string>
#include <unordered_map>
#include <functional>
#include "Sprite.h"
#include "Animation.h"

using namespace Cute;

// Enhanced sprite class with animation support
class AnimatedSprite : public Sprite {
private:
    std::string name;                                    // Sprite name
    std::unordered_map<std::string, Animation> animations; // Collection of animations
    Animation* currentAnimation;                         // Currently playing animation
    std::string currentAnimationName;                    // Name of current animation

    // Animation completion callback
    std::function<void(const std::string&)> onAnimationComplete;

public:
    // Constructors
    AnimatedSprite();
    AnimatedSprite(const char* texture_path);
    AnimatedSprite(const std::string& spriteName);
    AnimatedSprite(const std::string& spriteName, const char* texture_path);

    // Destructor
    ~AnimatedSprite();

    // Animation management
    void addAnimation(const Animation& animation);
    void removeAnimation(const std::string& animationName);
    bool playAnimation(const std::string& animationName);
    void stopAnimation();
    void pauseAnimation();
    void resumeAnimation();

    // Animation control
    void setAnimationSpeed(const std::string& animationName, float speed);
    void setAnimationLooping(const std::string& animationName, bool looping);
    void setOnAnimationComplete(std::function<void(const std::string&)> callback);

    // Animation state queries
    bool hasAnimation(const std::string& animationName) const;
    bool hasCurrentAnimation() const;
    const std::string& getCurrentAnimationName() const;
    int getCurrentFrameIndex() const;
    int getAnimationCount() const;
    const Animation& getCurrentAnimation() const;

    // Override Sprite methods
    void update(float dt) override;
    void render() override;

    // Getters
    const std::string& getName() const;

    // Setters
    void setName(const std::string& newName);

    // Utility
    void clearAnimations();
    void resetCurrentAnimation();
    float getAnimationProgress() const;
};
