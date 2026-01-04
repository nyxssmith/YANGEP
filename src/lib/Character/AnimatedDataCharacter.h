#ifndef ANIMATED_DATA_CHARACTER_H
#define ANIMATED_DATA_CHARACTER_H

#include <cute.h>
#include "SpriteAnimationLoader.h"
#include "DataFile.h"
#include "HitBox.h"
#include "Action.h"
#include "IVisualEffect.h"
#include "Damage.h"
#include <memory>
#include <vector>
#include <deque>
#include <functional>

using namespace Cute;

// Forward declaration
class LevelV1;
class HitBox;
class Action;
class IGhostTrailEffect;
class GhostTrailRenderer;

// Demo class to showcase the new SpriteAnimationLoader system
class AnimatedDataCharacter
{
public:
    AnimatedDataCharacter();
    virtual ~AnimatedDataCharacter();

    // Initialize the character with a folder path containing character.json
    bool init(const std::string &folderPath);

    // Update demo state with a move vector
    void update(float dt, v2 moveVector);

    // Render the demo
    void render();

    // Render the demo at a specific position
    void render(v2 renderPosition);

    // Visual FX: trigger an effect by name (replaces current effect)
    void triggerEffect(const std::string &name, int flashes = 3, float totalDuration = 2.0f, float maxIntensity = 0.85f);
    // Visual FX: trigger with completion callback
    void triggerEffect(const std::string &name, int flashes, float totalDuration, float maxIntensity, std::function<void()> onComplete);

    // deprecated
    void handleInput();

    // Check if demo is valid
    bool isValid() const;

    // Get current position
    v2 getPosition() const;

    // Set current position
    void setPosition(v2 newPosition);
    // Teleport at a safe point in the frame (applied at end of update)
    void queueTeleport(v2 target);

    // Get current direction
    Direction getCurrentDirection() const;

    // Set the level for agent queries
    void setLevel(LevelV1 *level);

    // Get the level
    LevelV1 *getLevel() const;

    // Get the datafile path
    const std::string &getDataFilePath() const;

    // Get hitbox in world coordinates
    HitBox *getHitbox() const;

    // Set hitbox visibility
    void sethitboxDebugActive(bool active);

    // Render only the action hitbox (separate from character rendering)
    void renderActionHitbox();

    // Action state
    void setDoingAction(bool doing);
    bool getIsDoingAction() const;
    void setActiveAction(Action *action);
    Action *getActiveAction() const;

    // ActionsList management
    bool addAction(const std::string &folderPath);
    bool removeAction(const std::string &actionName);
    const std::vector<Action> &getActions() const;

    // Action pointer management
    void setActionPointerA(size_t index);
    void setActionPointerB(size_t index);
    Action *getActionPointerA() const;
    Action *getActionPointerB() const;

    // Hit handling
    void OnHit(AnimatedDataCharacter *character, Damage damage);
    // Active ghost-trail (if any), nullptr otherwise
    IGhostTrailEffect *getActiveGhostTrailEffect() const;

private:
    // The animation loader
    SpriteAnimationLoader loader;

    // DataFile containing character configuration
    DataFile datafile;

    // Animation table containing all skeleton animations
    AnimationTable animationTable;

    // Current animation state
    std::string currentAnimation;
    Direction currentDirection;
    int currentFrame;
    float frameTimer;

    // Demo state
    bool initialized;
    float demoTime;

    // Input state
    bool keysPressed[4];   // UP, LEFT, DOWN, RIGHT
    bool animationKeys[2]; // 1 for idle, 2 for walkcycle

    // Demo parameters
    float directionChangeTime;
    float animationChangeTime;

    // Position for rendering
    v2 position;
    bool pendingTeleport = false;
    v2 teleportTarget = v2(0, 0);

    // Movement tracking for animation switching
    bool wasMoving;
    bool isDoingAction;

    // Hitbox state
    bool hitboxDebugActive;
    float hitboxSize;
    float hitboxDistance;
    HitBox *characterHitbox; // Character's footprint hitbox
    HitboxShape hitboxShape; // for constructor
    LevelV1 *level;          // Pointer to level for agent queries

    // ActionsList - list of actions for this character
    std::vector<Action> actionsList;
    size_t actionPointerA;
    size_t actionPointerB;
    Action *activeAction; // Currently active action

    // Helper methods
    void cycleDirection();
    void cycleAnimation();
    void updateAnimation(float dt);
    void renderCurrentFrame();
    void renderCurrentFrameAt(v2 renderPosition);
    void renderDebugInfo();
    void renderHitbox();

    // Visual effects
    std::deque<std::unique_ptr<IVisualEffect>> effectQueue;
    void beginFrontEffect();
    void endFrontEffect();

    // Allow renderer to access frame rendering helpers
    friend class GhostTrailRenderer;
};

#endif // ANIMATED_DATA_CHARACTER_H
