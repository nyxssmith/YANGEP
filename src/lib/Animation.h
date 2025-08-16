#pragma once

#include <cute.h>
#include <string>
#include <vector>
#include <functional>

using namespace Cute;

// Animation frame structure for 2D sprite sheet animations
struct AnimationFrame {
    int frameX;                    // X position in sprite sheet (0-based)
    int frameY;                    // Y position in sprite sheet (0-based)
    float duration;                // How long this frame should be displayed (in seconds)
    std::string frameName;         // Optional name for the frame

    AnimationFrame() : frameX(0), frameY(0), duration(0.1f), frameName("") {}
    AnimationFrame(int x, int y, float dur, const std::string& name = "")
        : frameX(x), frameY(y), duration(dur), frameName(name) {}
};

// Direction enum for skeleton sprites
enum class SkeletonDirection {
    UP = 0,
    LEFT = 1,
    DOWN = 2,
    RIGHT = 3
};

// Animation class for managing sprite sheet frame sequences
class Animation {
private:
    std::string name;                     // Animation name
    std::vector<AnimationFrame> frames;   // Sequence of frames
    bool playing;                         // Is animation currently playing
    bool looping;                         // Should animation loop when complete
    float speed;                          // Playback speed multiplier
    float currentTime;                    // Current time within the animation
    int currentFrameIndex;                // Current frame being displayed

    // Sprite sheet properties
    v2 frameSize;                         // Size of each frame (e.g., 64x256)
    v2 sheetSize;                         // Total size of sprite sheet

    // Callback for when animation completes
    std::function<void(const std::string&)> onCompleteCallback;

public:
    // Constructors
    Animation();
    Animation(const std::string& animName);
    Animation(const std::string& animName, v2 frameSize, v2 sheetSize);

    // Core methods
    void update(float deltaTime);
    void play();
    void pause();
    void stop();
    void reset();

    // Frame management
    void addFrame(const AnimationFrame& frame);
    void addFrame(int frameX, int frameY, float duration, const std::string& frameName = "");
    void addFrameSequence(int startFrame, int frameCount, float frameDuration, bool horizontal = true);
    void addDirectionalFrameSequence(int startFrame, int frameCount, float frameDuration, SkeletonDirection direction);
    void addWalkCycleForDirection(SkeletonDirection direction, float frameDuration = 0.1f);
    void removeFrame(int index);
    void clearFrames();

    // Getters
    const std::string& getName() const;
    int getFrameCount() const;
    int getCurrentFrame() const;
    const AnimationFrame& getFrame(int index) const;
    bool isPlaying() const;
    bool isLooping() const;
    float getSpeed() const;
    float getProgress() const; // 0.0 to 1.0

    // Sprite sheet properties
    v2 getFrameSize() const;
    v2 getSheetSize() const;
    v2 getCurrentFrameUV() const; // Returns UV coordinates for current frame

    // Setters
    void setName(const std::string& newName);
    void setLooping(bool shouldLoop);
    void setSpeed(float newSpeed);
    void setFrameSize(v2 size);
    void setSheetSize(v2 size);
    void setOnComplete(std::function<void(const std::string&)> callback);

    // Utility
    bool hasFrames() const;
    void reverse();
    void setFrameDuration(int frameIndex, float duration);
};
