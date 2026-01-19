#pragma once

#include "../FileHandling/DataFile.h"
#include "Damage.h"
#include <string>
#include <cute.h>

// Forward declarations
class HitBox;
class AnimatedDataCharacter;

class Action : public DataFile
{
private:
    DataFile hitboxData;
    bool hasHitbox;
    HitBox *hitbox;
    float hitboxSize;
    float hitboxDistance;
    bool isActive;
    AnimatedDataCharacter *character; // Pointer to the character this action belongs to

    // Timing members
    float warmup_timer;
    float cooldown_timer;
    bool in_cooldown;

    // Damage
    Damage *damage;
    bool hasDamage;

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
    void renderHitbox(CF_Color color, float border_opacity = 0.9f, float fill_opacity = 0.4f);

    // Character association
    void setCharacter(AnimatedDataCharacter *character);
    AnimatedDataCharacter *getCharacter() const;

    // Damage
    bool hasDamageData() const;
    Damage *getDamage() const;
    void doDamage();

    // Get characters currently in this action's hitbox
    std::vector<AnimatedDataCharacter *> getCharactersInHitbox() const;
};
