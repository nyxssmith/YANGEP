#include "Action.h"
#include "HitBox.h"
#include "AnimatedDataCharacter.h"
#include "LevelV1.h"
#include "Projectile.h"
#include "../UI/ColorUtils.h"
#include <cute.h>
#include <unordered_set>

// Constructor that takes a folder path and loads action.json from that folder
Action::Action(const std::string &folderPath) : hasHitbox(false), hitbox(nullptr), hitboxSize(32.0f), hitboxDistance(0.0f), isActive(false), character(nullptr), warmup_timer(0.0f), cooldown_timer(0.0f), in_cooldown(false), damage(nullptr), hasDamage(false), animation("melee_right_claw"), loopAnimation(false)
{
    loadFromFolder(folderPath, hitboxSize, hitboxDistance);
}

// Constructor with custom hitbox size and distance
Action::Action(const std::string &folderPath, float hitboxSize, float hitboxDistance)
    : hasHitbox(false), hitbox(nullptr), hitboxSize(hitboxSize), hitboxDistance(hitboxDistance), isActive(false), character(nullptr), warmup_timer(0.0f), cooldown_timer(0.0f), in_cooldown(false), damage(nullptr), hasDamage(false), animation("melee_right_claw"), loopAnimation(false)
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
    delete projectilesData;
}

// Copy constructor
Action::Action(const Action &other)
    : DataFile(other), // Copy base class
      actionAuxData(other.actionAuxData),
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
      hasDamage(other.hasDamage),
      animation(other.animation),
      loopAnimation(other.loopAnimation),
      max_uses(other.max_uses),
      remaining_uses(other.remaining_uses),
      type(other.type),
      projectilesData(other.projectilesData ? new ProjectilesData(*other.projectilesData) : nullptr)
{
    // Deep copy the hitbox if it exists
    if (other.hitbox && hasHitbox)
    {
        hitbox = HitBox::createHitBoxFromJson(actionAuxData, hitboxSize, hitboxDistance);
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
        delete projectilesData;
        projectilesData = nullptr;

        // Copy members
        actionAuxData = other.actionAuxData;
        hasHitbox = other.hasHitbox;
        hitboxSize = other.hitboxSize;
        hitboxDistance = other.hitboxDistance;
        isActive = other.isActive;
        character = other.character;
        warmup_timer = other.warmup_timer;
        cooldown_timer = other.cooldown_timer;
        in_cooldown = other.in_cooldown;
        hasDamage = other.hasDamage;
        animation = other.animation;
        loopAnimation = other.loopAnimation;
        max_uses = other.max_uses;
        remaining_uses = other.remaining_uses;
        type = other.type;
        if (other.projectilesData)
        {
            projectilesData = new ProjectilesData(*other.projectilesData);
        }

        // Deep copy the hitbox if it exists
        if (other.hitbox && hasHitbox)
        {
            hitbox = HitBox::createHitBoxFromJson(actionAuxData, hitboxSize, hitboxDistance);
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

    // load type
    if (actionLoaded && contains("type") && (*this)["type"].is_string())
    {
        std::string typeStr = (*this)["type"].get<std::string>();
        if (typeStr == "tile")
            type = tile;
        else if (typeStr == "projectile")
            type = projectile;
    }

    // Load animation name if present in action.json, otherwise keep the default
    if (actionLoaded && contains("animation") && (*this)["animation"].is_string())
    {
        animation = (*this)["animation"].get<std::string>();
    }

    // Load uses (defaults to 10) and start with all uses remaining
    max_uses = 10;
    if (actionLoaded && contains("uses") && (*this)["uses"].is_number_integer())
    {
        max_uses = (*this)["uses"].get<int>();
    }
    remaining_uses = max_uses;

    // Load loop_animation flag if present in action.json, otherwise keep the default (play once)
    if (actionLoaded && contains("loop_animation") && (*this)["loop_animation"].is_boolean())
    {
        loopAnimation = (*this)["loop_animation"].get<bool>();
    }

    // if type is tile
    if (type == tile)
    {
        // Try to load hitbox.json if it exists
        std::string hitboxJsonPath = normalizedPath + "hitbox.json";
        hasHitbox = actionAuxData.load(hitboxJsonPath);

        if (hasHitbox)
        {
            printf("Action: Loaded hitbox data from %s\n", hitboxJsonPath.c_str());

            // Clean up existing hitbox if any
            if (hitbox)
            {
                // TODO properly delete the hitbox, but rn it crashes
                // delete hitbox;
                hitbox = nullptr;
            }

            // Create HitBox from JSON data
            hitbox = HitBox::createHitBoxFromJson(actionAuxData, hitboxSize, hitboxDistance);

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
    }
    if (type == projectile)
    {
        std::string projectileJsonPath = normalizedPath + "projectiles.json";
        bool hasProj = actionAuxData.load(projectileJsonPath);
        if (!hasProj || !actionAuxData.contains("projectiles") || !actionAuxData["projectiles"].is_array())
        {
            printf("Action: Warning - Missing or invalid projectiles array in %s\n", projectileJsonPath.c_str());
            return false;
        }

        if (projectilesData == nullptr)
        {
            projectilesData = new ProjectilesData();
        }
        else
        {
            projectilesData->projectiles.clear();
        }

        for (const auto &projectileJson : actionAuxData["projectiles"])
        {
            if (!projectileJson.is_object() ||
                !projectileJson.contains("speed") || !projectileJson["speed"].is_number() ||
                !projectileJson.contains("angle") || !projectileJson["angle"].is_number() ||
                !projectileJson.contains("entity") || !projectileJson["entity"].is_string())
            {
                printf("Action: Warning - Invalid projectile entry in %s\n", projectileJsonPath.c_str());
                projectilesData->projectiles.clear();
                return false;
            }

            projectilesData->projectiles.push_back({projectileJson["speed"].get<float>(),
                                                    projectileJson["angle"].get<float>(),
                                                    projectileJson["entity"].get<std::string>()});
        }
    }
    // TODO handle projectile type
    return actionLoaded;
}

// Get hitbox data
const DataFile &Action::getHitboxData() const
{
    return actionAuxData;
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
        return;
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
    if (isActive)
    {
        printf("Action: Warning - Attempting to doAction while already active\n");
        return;
    }
    if (remaining_uses > 0)
    {
        // TODO dont decrement if holder of item is npc
        remaining_uses--;
        printf("Action: Remaining uses of action %s decremented, now %d\n", getAnimation().c_str(), remaining_uses);
    }
    else if (remaining_uses == 0)
    {
        // TODO flash UI or something
        printf("Action: Warning - No remaining uses for this action\n");
        return;
    } // set to -1 for unlimited uses

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

            if (type == tile)
                doTileActionInUpdate();
            if (type == projectile)
                doProjectileActionInUpdate();
            warmup_timer = 0.0f;
            in_cooldown = true;

            // Set cooldown timer (convert ms to seconds)
            float cooldownMs = contains("cooldown") ? (*this)["cooldown"].get<float>() : 0.0f;
            cooldown_timer = cooldownMs / 1000.0f;
            // printf("Action: Entering cooldown phase (%.3f ms)\n", cooldownMs);

            // Check if this action has a global cooldown and apply it to all actions
            float globalCooldownMs = getGlobalCooldown();
            if (globalCooldownMs > 0.0f && character)
            {
                float globalCooldownSeconds = globalCooldownMs / 1000.0f;
                character->applyGlobalCooldown(globalCooldownSeconds);
                printf("Action: Applied global cooldown (%.3f ms) to all actions\\n", globalCooldownMs);
            }
        }
    }
    // Handle cooldown phase
    else
    {
        cooldown_timer -= dt;
        printf("Action: Cooldown timer: %.3f seconds remaining for action %s\n", cooldown_timer, getAnimation().c_str());
        //  Check if cooldown is complete
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

void Action::doTileActionInUpdate()
{
    // Placeholder for tile action logic
    doDamage(getCharactersInHitbox());
}

void Action::doProjectileActionInUpdate()
{
    // start time
    auto start = std::chrono::high_resolution_clock::now();

    // Placeholder for projectile action logic
    // spawn a projectile here and add it to the level
    printf("sSpawning projectile\n");
    // for each projectile in projectilesData
    std::string entityPath; // path data for spawning
                            // Get tile coordinates of character doing action
    float tileX = this->character->getPosition().x;
    float tileY = this->character->getPosition().y;

    if (projectilesData)
    {
        for (auto &proj : projectilesData->projectiles)
        {
            // calculate the vector to give the projectile

            // get facing direction of the character
            Direction facing = this->character->getCurrentDirection();
            // TODO maybe? Or make it so the action stops motion of the character?
            // TODO get the characters move vector and add it to the projectile's initial move vector so it moves faster than self?
            // pass in args to get the final move vector for the projectile
            v2 moveVector = getProjectileMoveVector(facing, proj.speed, proj.angle);

            entityPath = "/assets/DataFiles/Entities/" + proj.entity;
            printf("Spawning %s\n", proj.entity.c_str());
            // Create the agent

            std::unique_ptr<Projectile> newProjectile = this->character->level->createProjectileFromFile(entityPath);

            // The level takes ownership; use the returned pointer from here on
            Projectile *projectileAgent = newProjectile ? this->character->level->addProjectile(std::move(newProjectile)) : nullptr;

            if (projectileAgent)
            {
                // Convert tile coordinates to world pixel coordinates
                // Multiply by tile dimensions to get world position
                float worldX = tileX * this->character->level->getTileWidth();
                float worldY = tileY * this->character->level->getTileHeight();

                // Set the projectile's initial position and movement vector based on the character's position and facing direction
                projectileAgent->setMoveVector(moveVector);
                // when projectile hits something, use this action to doDamage
                projectileAgent->setAction(this);
                // set the sender of the projectile to the character that spawned it
                projectileAgent->setSender(this->character);
                // set the initial position of the projectile in the world
                projectileAgent->setPosition(cf_v2(tileX, tileY));
                printf("Action:   Set projectile position to tile (%.1f, %.1f) = world (%.1f, %.1f)\n",
                       tileX, tileY, worldX, worldY);

                printf("Action:   Projectile '%s' created\n", entityPath.c_str());
            }
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;
    printf("Action: Projectile action update took %.3f ms\n", elapsed.count());
}

v2 Action::getProjectileMoveVector(Direction dir, float speed, float angle)
{
    // TODO handle angle
    // get the normalized movement vector based on the character's facing direction
    float projectileMoveX = 0.0f;
    float projectileMoveY = 0.0f;

    if (dir == Direction::LEFT)
    {
        projectileMoveX = -1.0f;
    }
    else if (dir == Direction::RIGHT)
    {
        projectileMoveX = 1.0f;
    }
    else if (dir == Direction::UP)
    {
        projectileMoveY = 1.0f;
    }
    else if (dir == Direction::DOWN)
    {
        projectileMoveY = -1.0f;
    }

    // apply the angle to the movement vector
    if (angle != 0.0f)
    {
        float radians = angle * (3.14159265f / 180.0f);
        float cosAngle = cosf(radians);
        float sinAngle = sinf(radians);
        float rotatedX = projectileMoveX * cosAngle - projectileMoveY * sinAngle;
        float rotatedY = projectileMoveX * sinAngle + projectileMoveY * cosAngle;
        projectileMoveX = rotatedX;
        projectileMoveY = rotatedY;
    }

    return cf_v2(projectileMoveX * speed, projectileMoveY * speed);
}

void Action::renderHitbox(CF_Color color, float border_opacity, float fill_opacity)
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
    hitbox->render(characterPosition, facingDirection, *level, color, border_opacity, fill_opacity);
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

const std::string &Action::getAnimation() const
{
    return animation;
}

bool Action::getLoopAnimation() const
{
    return loopAnimation;
}

std::vector<AnimatedDataCharacter *> Action::getCharactersInHitbox()
{
    // Get all characters in the hitbox
    std::vector<AnimatedDataCharacter *> charactersInHitboxList; // = getCharactersInHitbox();

    // Use a set to prevent duplicates
    std::unordered_set<AnimatedDataCharacter *> uniqueCharacters(
        charactersInHitboxList.begin(),
        charactersInHitboxList.end());
    // Convert the set back to a vector
    std::vector<AnimatedDataCharacter *> uniqueCharactersList(
        uniqueCharacters.begin(),
        uniqueCharacters.end());

    return uniqueCharactersList;
}

void Action::doDamage(std::vector<AnimatedDataCharacter *> toDamage)
{
    // Only do damage if we have damage data
    if (!hasDamage || !damage)
    {
        return;
    }

    // Call OnHit for each unique character
    for (AnimatedDataCharacter *hitCharacter : toDamage)
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

float Action::getGlobalCooldown() const
{
    // Return global_cooldown value in milliseconds, or 0 if not set
    if (contains("global_cooldown") && (*this)["global_cooldown"].is_number())
    {
        return (*this)["global_cooldown"].get<float>();
    }
    return 0.0f;
}

void Action::applyCooldown(float cooldownSeconds)
{
    // Only apply if we're in cooldown and the new cooldown is less than current
    if (in_cooldown && cooldownSeconds < cooldown_timer)
    {
        cooldown_timer = cooldownSeconds;
    }
    // If not in cooldown but cooldown > 0, start cooldown
    else if (!in_cooldown && cooldownSeconds > 0.0f)
    {
        in_cooldown = true;
        cooldown_timer = cooldownSeconds;
        isActive = true;     // Activate the action so it gets updated and cooldown ticks down
        warmup_timer = 0.0f; // Skip warmup phase since we're going straight to cooldown
    }
}
