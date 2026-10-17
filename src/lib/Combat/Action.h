#pragma once

#include "../FileHandling/DataFile.h"
#include "Damage.h"
#include <string>
#include <cute.h>
#include "SpriteAnimationLoader.h"

enum ActionType
{
    tile,
    projectile
};

// Forward declarations
class HitBox;
class AnimatedDataCharacter;
struct ProjectilesData;

class Action : public DataFile
{
private:
    DataFile actionAuxData;
    bool hasHitbox;
    HitBox *hitbox;
    float hitboxSize = 32.0f;
    float hitboxDistance;
    bool isActive = false;
    AnimatedDataCharacter *character; // Pointer to the character this action belongs to

    // Timing members
    float warmup_timer = 0.0f;
    float cooldown_timer = 0.0f;
    bool in_cooldown = false;

    // Damage
    Damage *damage;
    bool hasDamage = false;

    // Animation to play while this action is executing
    std::string animation;
    bool loopAnimation = false;
    int max_uses = 10;
    int remaining_uses = 10;
    ActionType type = tile;
    ProjectilesData *projectilesData = nullptr; // only set for projectile actions

public:
    Action() = default;
    Action(const std::string &folderPath);
    Action(const std::string &folderPath, float hitboxSize, float hitboxDistance);
    ~Action();

    // Copy constructor and copy assignment operator for deep copying
    Action(const Action &other);
    Action &operator=(const Action &other);

    // Load action.json from a folder path
    bool loadFromFolder(const std::string &folderPath);
    bool loadFromFolder(const std::string &folderPath, float hitboxSize, float hitboxDistance);

    // Get hitbox data
    const DataFile &getHitboxData() const;
    bool hasHitboxData() const;
    HitBox *getHitBox() const;

    // Action state
    void setActive(bool active);
    bool getIsActive() const;
    float getWarmupTimer() const;
    bool getInCooldown() const;
    void doAction();
    void update(float dt);
    void doTileActionInUpdate();
    void doProjectileActionInUpdate();
    Cute::v2 getProjectileMoveVector(Direction dir, float speed, float angle);
    void renderHitbox(CF_Color color, float border_opacity = 0.9f, float fill_opacity = 0.4f);

    // Character association
    void setCharacter(AnimatedDataCharacter *character);
    AnimatedDataCharacter *getCharacter() const;

    // Damage
    bool hasDamageData() const;
    Damage *getDamage() const;
    void doDamage(std::vector<AnimatedDataCharacter *> toDamage);
    std::vector<AnimatedDataCharacter *> getCharactersInHitbox();

    // Animation to play while this action is executing (defaults to "melee_right_claw")
    const std::string &getAnimation() const;
    // Whether the action's animation should loop instead of playing once (defaults to false)
    bool getLoopAnimation() const;

    // Get characters currently in this action's hitbox
    std::vector<AnimatedDataCharacter *> getCharactersInHitbox() const;

    // Global cooldown support
    float getGlobalCooldown() const;           // Get global_cooldown value from action.json (in ms, 0 if not set)
    void applyCooldown(float cooldownSeconds); // Apply cooldown only if less than current cooldown

    int getMaxUses() const { return max_uses; }
    int getRemainingUses() const { return remaining_uses; }

    ActionType getType() const { return type; }
};
