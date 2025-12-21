#include "Action.h"
#include "HitBox.h"
#include "AnimatedDataCharacter.h"
#include "LevelV1.h"
#include <cute.h>

// Constructor that takes a folder path and loads action.json from that folder
Action::Action(const std::string &folderPath) : hasHitbox(false), hitbox(nullptr), hitboxSize(32.0f), hitboxDistance(0.0f), isActive(false), character(nullptr)
{
    loadFromFolder(folderPath, hitboxSize, hitboxDistance);
}

// Constructor with custom hitbox size and distance
Action::Action(const std::string &folderPath, float hitboxSize, float hitboxDistance)
    : hasHitbox(false), hitbox(nullptr), hitboxSize(hitboxSize), hitboxDistance(hitboxDistance), isActive(false), character(nullptr)
{
    loadFromFolder(folderPath, hitboxSize, hitboxDistance);
}

// Destructor
Action::~Action()
{
    if (hitbox)
    {
        delete hitbox;
        hitbox = nullptr;
    }
}

// Copy constructor
Action::Action(const Action &other)
    : DataFile(other), // Copy base class
      hitboxData(other.hitboxData),
      hasHitbox(other.hasHitbox),
      hitbox(nullptr),
      hitboxSize(other.hitboxSize),
      hitboxDistance(other.hitboxDistance),
      isActive(other.isActive),
      character(other.character)
{
    // Deep copy the hitbox if it exists
    if (other.hitbox && hasHitbox)
    {
        hitbox = HitBox::createHitBoxFromJson(hitboxData, hitboxSize, hitboxDistance);
    }
}

// Copy assignment operator
Action &Action::operator=(const Action &other)
{
    if (this != &other)
    {
        // Copy base class
        DataFile::operator=(other);

        // Clean up existing hitbox
        if (hitbox)
        {
            delete hitbox;
            hitbox = nullptr;
        }

        // Copy members
        hitboxData = other.hitboxData;
        hasHitbox = other.hasHitbox;
        hitboxSize = other.hitboxSize;
        hitboxDistance = other.hitboxDistance;
        isActive = other.isActive;
        character = other.character;

        // Deep copy the hitbox if it exists
        if (other.hitbox && hasHitbox)
        {
            hitbox = HitBox::createHitBoxFromJson(hitboxData, hitboxSize, hitboxDistance);
        }
    }
    return *this;
}

// Load action.json from a folder path
bool Action::loadFromFolder(const std::string &folderPath)
{
    return loadFromFolder(folderPath, hitboxSize, hitboxDistance);
}

// Load action.json from a folder path with custom hitbox size
bool Action::loadFromFolder(const std::string &folderPath, float hitboxSize, float hitboxDistance)
{
    this->hitboxSize = hitboxSize;
    this->hitboxDistance = hitboxDistance;

    // Ensure the folder path ends with a separator
    std::string normalizedPath = folderPath;
    if (!normalizedPath.empty() && normalizedPath.back() != '/')
    {
        normalizedPath += '/';
    }

    // Construct the full path to action.json
    std::string actionJsonPath = normalizedPath + "action.json";

    // Load the JSON file using the base class method
    bool actionLoaded = load(actionJsonPath);

    // Try to load hitbox.json if it exists
    std::string hitboxJsonPath = normalizedPath + "hitbox.json";
    hasHitbox = hitboxData.load(hitboxJsonPath);

    if (hasHitbox)
    {
        printf("Action: Loaded hitbox data from %s\n", hitboxJsonPath.c_str());

        // Clean up existing hitbox if any
        if (hitbox)
        {
            delete hitbox;
            hitbox = nullptr;
        }

        // Create HitBox from JSON data
        hitbox = HitBox::createHitBoxFromJson(hitboxData, hitboxSize, hitboxDistance);

        if (hitbox)
        {
            printf("Action: Created HitBox with %zu tiles\n", hitbox->getTiles().size());
        }
        else
        {
            printf("Action: Warning - Failed to create HitBox from JSON data\n");
            hasHitbox = false;
        }
    }

    return actionLoaded;
}

// Get hitbox data
const DataFile &Action::getHitboxData() const
{
    return hitboxData;
}

bool Action::hasHitboxData() const
{
    return hasHitbox;
}

HitBox *Action::getHitBox() const
{
    return hitbox;
}

void Action::setActive(bool active)
{
    isActive = active;

    // Notify the character if one is associated
    if (character)
    {
        if (active)
        {
            character->setDoingAction(true);
            character->setActiveAction(this);
        }
        else
        {
            character->setDoingAction(false);
            character->setActiveAction(nullptr);
        }
    }
}

bool Action::getIsActive() const
{
    return isActive;
}

void Action::doAction()
{
    // For now, just set the action to active
    // Later this will include checks if the action can be performed
    setActive(true);
}

void Action::update(float dt)
{
    // For now, empty - will be implemented later
    // print test
    printf("Action: Updating action (dt = %.3f)\n", dt);
}

void Action::renderHitbox()
{
    // Check if we have a hitbox and character to render
    if (!hasHitbox || !hitbox || !character)
    {
        return;
    }

    // Get the level from the character
    LevelV1 *level = character->getLevel();
    if (!level)
    {
        printf("Action: Cannot render hitbox - character has no level\n");
        return;
    }

    // Get character's current position and direction
    v2 characterPosition = character->getPosition();
    Direction facingDirection = character->getCurrentDirection();

    // Call hitbox render with character info and level
    hitbox->render(characterPosition, facingDirection, *level);
}

void Action::setCharacter(AnimatedDataCharacter *character)
{
    this->character = character;
}

AnimatedDataCharacter *Action::getCharacter() const
{
    return character;
}
