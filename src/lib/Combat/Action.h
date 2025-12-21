#pragma once

#include "../FileHandling/DataFile.h"
#include <string>

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
    void doAction();
    void update(float dt);
    void renderHitbox();

    // Character association
    void setCharacter(AnimatedDataCharacter *character);
    AnimatedDataCharacter *getCharacter() const;
};
