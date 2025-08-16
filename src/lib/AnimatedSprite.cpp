#include "AnimatedSprite.h"
#include <algorithm>

// Default constructor
AnimatedSprite::AnimatedSprite()
    : name("unnamed")
    , currentAnimation(nullptr)
    , currentAnimationName("")
{
}

// Constructor with texture path
AnimatedSprite::AnimatedSprite(const char* texture_path)
    : Sprite(texture_path)
    , name("unnamed")
    , currentAnimation(nullptr)
    , currentAnimationName("")
{
}

// Constructor with name
AnimatedSprite::AnimatedSprite(const std::string& spriteName)
    : Sprite()  // Call base Sprite constructor to initialize sprite handle
    , name(spriteName)
    , currentAnimation(nullptr)
    , currentAnimationName("")
{
}

// Constructor with name and texture path
AnimatedSprite::AnimatedSprite(const std::string& spriteName, const char* texture_path)
    : Sprite(texture_path)
    , name(spriteName)
    , currentAnimation(nullptr)
    , currentAnimationName("")
{
}

// Destructor
AnimatedSprite::~AnimatedSprite() {
    // Cleanup handled automatically
}

// Animation management
void AnimatedSprite::addAnimation(const Animation& animation) {
    animations[animation.getName()] = animation;
}

void AnimatedSprite::removeAnimation(const std::string& animationName) {
    auto it = animations.find(animationName);
    if (it != animations.end()) {
        // If this was the current animation, stop it
        if (currentAnimationName == animationName) {
            stopAnimation();
        }
        animations.erase(it);
    }
}

bool AnimatedSprite::playAnimation(const std::string& animationName) {
    auto it = animations.find(animationName);
    if (it != animations.end()) {
        // Stop current animation if different
        if (currentAnimationName != animationName) {
            stopAnimation();
        }

        currentAnimation = &(it->second);
        currentAnimationName = animationName;
        currentAnimation->play();
        return true;
    }
    return false;
}

void AnimatedSprite::stopAnimation() {
    if (currentAnimation) {
        currentAnimation->stop();
    }
    currentAnimation = nullptr;
    currentAnimationName = "";
}

void AnimatedSprite::pauseAnimation() {
    if (currentAnimation) {
        currentAnimation->pause();
    }
}

void AnimatedSprite::resumeAnimation() {
    if (currentAnimation) {
        currentAnimation->play();
    }
}

// Animation control
void AnimatedSprite::setAnimationSpeed(const std::string& animationName, float speed) {
    auto it = animations.find(animationName);
    if (it != animations.end()) {
        it->second.setSpeed(speed);
    }
}

void AnimatedSprite::setAnimationLooping(const std::string& animationName, bool looping) {
    auto it = animations.find(animationName);
    if (it != animations.end()) {
        it->second.setLooping(looping);
    }
}

void AnimatedSprite::setOnAnimationComplete(std::function<void(const std::string&)> callback) {
    onAnimationComplete = callback;
}

// Animation state queries
bool AnimatedSprite::hasAnimation(const std::string& animationName) const {
    return animations.find(animationName) != animations.end();
}

bool AnimatedSprite::hasCurrentAnimation() const {
    return currentAnimation != nullptr;
}

const std::string& AnimatedSprite::getCurrentAnimationName() const {
    return currentAnimationName;
}

int AnimatedSprite::getCurrentFrameIndex() const {
    if (currentAnimation) {
        return currentAnimation->getCurrentFrame();
    }
    return 0;
}

int AnimatedSprite::getAnimationCount() const {
    return animations.size();
}

const Animation& AnimatedSprite::getCurrentAnimation() const {
    static const Animation emptyAnimation;
    if (currentAnimation) {
        return *currentAnimation;
    }
    return emptyAnimation;
}

// Override Sprite methods
void AnimatedSprite::update(float dt) {
    // Update current animation
    if (currentAnimation) {
        currentAnimation->update(dt);

        // Check if animation completed and call callback
        if (!currentAnimation->isPlaying() && onAnimationComplete) {
            onAnimationComplete(currentAnimationName);
        }
    }

    // Call base class update
    Sprite::update(dt);
}

void AnimatedSprite::render() {
    // For now, just render the base sprite
    // TODO: Implement sprite sheet UV mapping based on current animation frame
    Sprite::render();
}

// Getters
const std::string& AnimatedSprite::getName() const {
    return name;
}

// Setters
void AnimatedSprite::setName(const std::string& newName) {
    name = newName;
}

// Utility
void AnimatedSprite::clearAnimations() {
    animations.clear();
    stopAnimation();
}

void AnimatedSprite::resetCurrentAnimation() {
    if (currentAnimation) {
        currentAnimation->reset();
    }
}

float AnimatedSprite::getAnimationProgress() const {
    if (currentAnimation) {
        return currentAnimation->getProgress();
    }
    return 0.0f;
}
