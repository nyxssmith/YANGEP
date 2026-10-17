#ifndef ANIMATED_DATA_CHARACTER_H
#define ANIMATED_DATA_CHARACTER_H

#include <cute.h>
#include <cute_math.h>
#include "SpriteAnimationLoader.h"
#include "DataFile.h"
#include "HitBox.h"
#include "Action.h"
#include "IVisualEffect.h"
#include "Damage.h"
#include "../Items/Inventory.h"
#include <memory>
#include <vector>
#include <deque>
#include <functional>

using namespace Cute;

// Stage of life enum
enum class StageOfLife
{
    Alive,
    Dying,
    Dead
};

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

    // Set the scene-owned loader used to fetch animations. Must be called before init().
    void setSpriteAnimationLoader(SpriteAnimationLoader *spriteLoader) { loader = spriteLoader; }

    // Initialize the character with a folder path containing character.json
    virtual bool init(const std::string &folderPath);

    // Update demo state with a move vector
    void update(float dt, v2 moveVector);

    // Hook for ticking actions owned by items rather than by actionsList
    virtual void updateItemActions(float dt) {}

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

    // Set visual sprite scale relative to the tilemap.
    void setEntityScale(float scale);

    // Get this entity's own scale multiplier (from character.json, default 1.0), independent of the global scale
    float getEntityScaleMultiplier() const;

    // Get current direction
    Direction getCurrentDirection() const;

    // Set current direction
    void setDirection(Direction direction);

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
    bool addActionDirectly(Action &action);
    // Action pointer management
    virtual void setActionPointerA(size_t index);
    virtual void setActionPointerB(size_t index);
    Action *getActionPointerA() const;
    Action *getActionPointerB() const;

    // Apply global cooldown to all actions
    virtual void applyGlobalCooldown(float globalCooldownSeconds);

    // Hit handling
    void OnHit(AnimatedDataCharacter *character, Damage damage);
    // Active ghost-trail (if any), nullptr otherwise
    IGhostTrailEffect *getActiveGhostTrailEffect() const;

    // Stage of life management
    void setStageOfLife(StageOfLife stage);
    StageOfLife getStageOfLife() const;

    // Inventory access
    Inventory &getInventory();
    const Inventory &getInventory() const;

    // Hitbox accessor
    HitBox *getCharacterHitbox() const;

    // Initialization state
    bool getIsInitialized() const;

    // Input state accessors
    bool getKeysPressed(int index) const;
    void setKeysPressed(int index, bool pressed);
    bool getAnimationKeys(int index) const;
    void setAnimationKeys(int index, bool pressed);
    // projectiles need access to these
    Direction currentDirection;
    LevelV1 *level; // Pointer to level for agent queries

    // inventory management to be used by crabs
    virtual bool addItemToInventory(std::string fullPath);

    bool is_player = false;

    // health system and defaults
    int currentHealth = 1;
    int maxHealth = 1;
    int currentShields = 0;
    int maxShields = 0;
    float timeBetweenShieldRecharge = 10.0f;
    float shieldRechargerTimer = 0.0f;

    void updateShieldRecharge(float dt);

protected:
    // Inventory - character's item inventory
    Inventory inventory;

private:
    // Shared animation loader/cache owned by the scene (not owned here)
    SpriteAnimationLoader *loader = nullptr;

    // DataFile containing character configuration
    DataFile datafile;

    // Animation table shared between all characters using the same layers
    std::shared_ptr<const AnimationTable> animationTable;

    // Current animation state
    std::string currentAnimation;
    int currentFrame;
    float frameTimer;
    bool currentAnimationLoops; // false holds on the last frame instead of looping back to 0

    // Demo state
    bool initialized;
    float demoTime;

protected:
    // Set initialized state (for derived classes)
    void setInitialized(bool value);

    // Input state
    bool keysPressed[4];   // UP, LEFT, DOWN, RIGHT
    bool animationKeys[2]; // 1 for idle, 2 for walkcycle

    // Demo parameters
    float directionChangeTime;
    float animationChangeTime;

    // Position for rendering
    v2 position;
    float entityScale;           // Global scale, set externally (e.g. from window-config.json)
    float entityScaleMultiplier; // Per-entity scale, from character.json (default 1.0)

    // Movement tracking for animation switching
    bool wasMoving;
    bool isDoingAction;

    // Stage of life
    StageOfLife stageOfLife;

    // Hitbox state
    bool hitboxDebugActive;
    float hitboxSize;
    float hitboxDistance;
    HitBox *characterHitbox; // Character's footprint hitbox
    HitboxShape hitboxShape; // for constructor

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
