#include "Action.h"
#include "HitBox.h"
#include "AnimatedDataCharacter.h"
#include "LevelV1.h"
#include "../UI/ColorUtils.h"
#include <cute.h>
#include <unordered_set>

// Constructor that takes a folder path and loads action.json from that folder
Action::Action(const std::string &folderPath) : hasHitbox(false), hitbox(nullptr), hitboxSize(32.0f), hitboxDistance(0.0f), isActive(false), character(nullptr), warmup_timer(0.0f), cooldown_timer(0.0f), in_cooldown(false), damage(nullptr), hasDamage(false)
{
    loadFromFolder(folderPath, hitboxSize, hitboxDistance);
}

// Constructor with custom hitbox size and distance
Action::Action(const std::string &folderPath, float hitboxSize, float hitboxDistance)
    : hasHitbox(false), hitbox(nullptr), hitboxSize(hitboxSize), hitboxDistance(hitboxDistance), isActive(false), character(nullptr), warmup_timer(0.0f), cooldown_timer(0.0f), in_cooldown(false), damage(nullptr), hasDamage(false)
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
    if (damage)
    {
        delete damage;
        damage = nullptr;
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
      character(other.character),
      warmup_timer(other.warmup_timer),
      cooldown_timer(other.cooldown_timer),
      in_cooldown(other.in_cooldown),
      damage(nullptr),
      hasDamage(other.hasDamage)
{
    // Deep copy the hitbox if it exists
    if (other.hitbox && hasHitbox)
    {
        hitbox = HitBox::createHitBoxFromJson(hitboxData, hitboxSize, hitboxDistance);
    }
    // Deep copy the damage if it exists
    if (other.damage && hasDamage)
    {
        damage = new Damage(*other.damage);
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

        // Clean up existing damage
        if (damage)
        {
            delete damage;
            damage = nullptr;
        }

        // Copy members
        hitboxData = other.hitboxData;
        hasHitbox = other.hasHitbox;
        hitboxSize = other.hitboxSize;
        hitboxDistance = other.hitboxDistance;
        isActive = other.isActive;
        character = other.character;
        warmup_timer = other.warmup_timer;
        cooldown_timer = other.cooldown_timer;
        in_cooldown = other.in_cooldown;
        hasDamage = other.hasDamage;

        // Deep copy the hitbox if it exists
        if (other.hitbox && hasHitbox)
        {
            hitbox = HitBox::createHitBoxFromJson(hitboxData, hitboxSize, hitboxDistance);
        }

        // Deep copy the damage if it exists
        if (other.damage && hasDamage)
        {
            damage = new Damage(*other.damage);
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

    // Load damage if present in action.json
    if (actionLoaded && contains("damage") && (*this)["damage"].is_number())
    {
        float damageValue = (*this)["damage"].get<float>();
        damage = new Damage(damageValue);
        hasDamage = true;
        printf("Action: Loaded damage value: %.2f\n", damageValue);
    }
    else
    {
        hasDamage = false;
        damage = nullptr;
    }

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
    // Debug if trying to activate while in cooldown
    if (active && in_cooldown)
    {
        printf("Action: Warning - Attempting to activate action while in cooldown (%.3f seconds remaining)\n", cooldown_timer);
    }

    isActive = active;

    // Reset timers when activating
    if (active)
    {
        warmup_timer = 0.0f;
        cooldown_timer = 0.0f;
        in_cooldown = false;
    }

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

float Action::getWarmupTimer() const
{
    return warmup_timer;
}

bool Action::getInCooldown() const
{
    return in_cooldown;
}

void Action::doAction()
{
    // For now, just set the action to active
    // Later this will include checks if the action can be performed
    setActive(true);
}

void Action::update(float dt)
{
    if (!isActive)
    {
        return;
    }

    // Handle warmup phase
    if (!in_cooldown)
    {
        warmup_timer += dt;

        // Check if warmup is complete (convert ms to seconds)
        float warmupMs = contains("warmup") ? (*this)["warmup"].get<float>() : 0.0f;
        if (warmup_timer >= warmupMs / 1000.0f)
        {
            printf("Action: Warmup complete (%.3f ms)\n", warmupMs);

            // Do damage when warmup completes
            doDamage();

            warmup_timer = 0.0f;
            in_cooldown = true;

            // Set cooldown timer (convert ms to seconds)
            float cooldownMs = contains("cooldown") ? (*this)["cooldown"].get<float>() : 0.0f;
            cooldown_timer = cooldownMs / 1000.0f;
        }
    }
    // Handle cooldown phase
    else
    {
        cooldown_timer -= dt;

        // Check if cooldown is complete
        if (cooldown_timer <= 0.0f)
        {
            // Reset everything
            setActive(false);
            warmup_timer = 0.0f;
            cooldown_timer = 0.0f;
            in_cooldown = false;
        }
    }
}

void Action::renderHitbox(CF_Color color)
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
        return;
    }

    // Get character's current position and direction
    v2 characterPosition = character->getPosition();
    Direction facingDirection = character->getCurrentDirection();

    // Call hitbox render with character info, level, and color
    hitbox->render(characterPosition, facingDirection, *level, color);
}

void Action::setCharacter(AnimatedDataCharacter *character)
{
    this->character = character;
}

AnimatedDataCharacter *Action::getCharacter() const
{
    return character;
}

bool Action::hasDamageData() const
{
    return hasDamage;
}

Damage *Action::getDamage() const
{
    return damage;
}

void Action::doDamage()
{
    // Only do damage if we have damage data
    if (!hasDamage || !damage)
    {
        return;
    }

    // Get all characters in the hitbox
    std::vector<AnimatedDataCharacter *> charactersInHitboxList = getCharactersInHitbox();

    // Use a set to prevent duplicates
    std::unordered_set<AnimatedDataCharacter *> uniqueCharacters(
        charactersInHitboxList.begin(),
        charactersInHitboxList.end());

    // Call OnHit for each unique character
    for (AnimatedDataCharacter *hitCharacter : uniqueCharacters)
    {
        if (hitCharacter)
        {
            hitCharacter->OnHit(character, *damage);
        }
    }
}

std::vector<AnimatedDataCharacter *> Action::getCharactersInHitbox() const
{
    std::vector<AnimatedDataCharacter *> charactersInHitbox;

    // Check if we have a character
    if (!character)
    {
        return charactersInHitbox;
    }

    // Get the level from the character
    LevelV1 *level = character->getLevel();
    if (!level)
    {
        return charactersInHitbox;
    }

    // Ask the level for all characters in this action's hitbox
    // Exclude the character performing the action
    return level->getCharactersInActionHitbox(this, character);
}
