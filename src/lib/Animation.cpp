#include "Animation.h"
#include <algorithm>

// Default constructor
Animation::Animation()
    : name("unnamed")
    , playing(false)
    , looping(true)
    , speed(1.0f)
    , currentTime(0.0f)
    , currentFrameIndex(0)
    , frameSize(v2(64, 256))  // Default frame size based on skeleton assets
    , sheetSize(v2(64, 256))  // Default sheet size
{
}

// Constructor with name
Animation::Animation(const std::string& animName)
    : name(animName)
    , playing(false)
    , looping(true)
    , speed(1.0f)
    , currentTime(0.0f)
    , currentFrameIndex(0)
    , frameSize(v2(64, 256))  // Default frame size based on skeleton assets
    , sheetSize(v2(64, 256))  // Default sheet size
{
}

// Constructor with name and sprite sheet properties
Animation::Animation(const std::string& animName, v2 frameSize, v2 sheetSize)
    : name(animName)
    , playing(false)
    , looping(true)
    , speed(1.0f)
    , currentTime(0.0f)
    , currentFrameIndex(0)
    , frameSize(frameSize)
    , sheetSize(sheetSize)
{
}

// Core methods
void Animation::update(float deltaTime) {
    if (!playing || frames.empty()) return;

    // Apply speed multiplier
    float adjustedDelta = deltaTime * speed;
    currentTime += adjustedDelta;

    // Check if we need to advance to next frame
    while (currentTime >= frames[currentFrameIndex].duration) {
        currentTime -= frames[currentFrameIndex].duration;
        currentFrameIndex++;

        // Check if animation is complete
        if (currentFrameIndex >= frames.size()) {
            if (looping) {
                // Loop back to beginning
                currentFrameIndex = 0;
            } else {
                // Stop at last frame
                currentFrameIndex = frames.size() - 1;
                playing = false;

                // Call completion callback if set
                if (onCompleteCallback) {
                    onCompleteCallback(name);
                }
                return;
            }
        }
    }
}

void Animation::play() {
    if (!frames.empty()) {
        playing = true;
    }
}

void Animation::pause() {
    playing = false;
}

void Animation::stop() {
    playing = false;
    reset();
}

void Animation::reset() {
    currentTime = 0.0f;
    currentFrameIndex = 0;
}

// Frame management
void Animation::addFrame(const AnimationFrame& frame) {
    frames.push_back(frame);
}

void Animation::addFrame(int frameX, int frameY, float duration, const std::string& frameName) {
    frames.emplace_back(frameX, frameY, duration, frameName);
}

void Animation::addFrameSequence(int startFrame, int frameCount, float frameDuration, bool horizontal) {
    for (int i = 0; i < frameCount; i++) {
        int frameX, frameY;

        if (horizontal) {
            // Frames are arranged horizontally (like walk cycle: 9 frames × 64 pixels)
            frameX = startFrame + i;
            frameY = 0;
        } else {
            // Frames are arranged vertically
            frameX = 0;
            frameY = startFrame + i;
        }

        std::string frameName = "frame_" + std::to_string(startFrame + i);
        frames.emplace_back(frameX, frameY, frameDuration, frameName);
    }
}

void Animation::addDirectionalFrameSequence(int startFrame, int frameCount, float frameDuration, SkeletonDirection direction) {
    int directionRow = static_cast<int>(direction);

    for (int i = 0; i < frameCount; i++) {
        int frameX = startFrame + i;
        int frameY = directionRow;

        std::string frameName = "frame_" + std::to_string(startFrame + i) + "_" + std::to_string(directionRow);
        frames.emplace_back(frameX, frameY, frameDuration, frameName);
    }
}

void Animation::addWalkCycleForDirection(SkeletonDirection direction, float frameDuration) {
    // Add 9 frames for walk cycle in the specified direction
    addDirectionalFrameSequence(0, 9, frameDuration, direction);
}

void Animation::removeFrame(int index) {
    if (index >= 0 && index < frames.size()) {
        frames.erase(frames.begin() + index);

        // Adjust current frame index if necessary
        if (currentFrameIndex >= frames.size()) {
            currentFrameIndex = std::max(0, (int)frames.size() - 1);
        }
    }
}

void Animation::clearFrames() {
    frames.clear();
    reset();
}

// Getters
const std::string& Animation::getName() const {
    return name;
}

int Animation::getFrameCount() const {
    return frames.size();
}

int Animation::getCurrentFrame() const {
    return currentFrameIndex;
}

const AnimationFrame& Animation::getFrame(int index) const {
    static const AnimationFrame emptyFrame;
    if (index >= 0 && index < frames.size()) {
        return frames[index];
    }
    return emptyFrame;
}

bool Animation::isPlaying() const {
    return playing;
}

bool Animation::isLooping() const {
    return looping;
}

float Animation::getSpeed() const {
    return speed;
}

float Animation::getProgress() const {
    if (frames.empty()) return 0.0f;

    float totalDuration = 0.0f;
    for (const auto& frame : frames) {
        totalDuration += frame.duration;
    }

    if (totalDuration <= 0.0f) return 0.0f;

    float currentProgress = 0.0f;
    for (int i = 0; i < currentFrameIndex; i++) {
        currentProgress += frames[i].duration;
    }
    currentProgress += currentTime;

    return currentProgress / totalDuration;
}

// Sprite sheet properties
v2 Animation::getFrameSize() const {
    return frameSize;
}

v2 Animation::getSheetSize() const {
    return sheetSize;
}

v2 Animation::getCurrentFrameUV() const {
    if (frames.empty() || currentFrameIndex >= frames.size()) {
        return v2(0, 0);
    }

    const AnimationFrame& currentFrame = frames[currentFrameIndex];

    // Calculate UV coordinates for the current frame
    // UV coordinates are normalized (0.0 to 1.0)
    float u = (currentFrame.frameX * frameSize.x) / sheetSize.x;
    float v = (currentFrame.frameY * frameSize.y) / sheetSize.y;

    return v2(u, v);
}

// Setters
void Animation::setName(const std::string& newName) {
    name = newName;
}

void Animation::setLooping(bool shouldLoop) {
    looping = shouldLoop;
}

void Animation::setSpeed(float newSpeed) {
    speed = std::max(0.0f, newSpeed); // Speed cannot be negative
}

void Animation::setFrameSize(v2 size) {
    frameSize = size;
}

void Animation::setSheetSize(v2 size) {
    sheetSize = size;
}

void Animation::setOnComplete(std::function<void(const std::string&)> callback) {
    onCompleteCallback = callback;
}

// Utility
bool Animation::hasFrames() const {
    return !frames.empty();
}

void Animation::reverse() {
    std::reverse(frames.begin(), frames.end());
    reset();
}

void Animation::setFrameDuration(int frameIndex, float duration) {
    if (frameIndex >= 0 && frameIndex < frames.size()) {
        frames[frameIndex].duration = std::max(0.0f, duration);
    }
}
