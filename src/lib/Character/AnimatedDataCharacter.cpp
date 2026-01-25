#include <cute.h>
#include <cute_draw.h>
#include <cute_math.h>
#include <spng.h>
#include "AnimatedDataCharacter.h"
#include "DataFile.h"
#include "LevelV1.h"
#include "HitBox.h"
#include "../UI/ColorUtils.h"
#include "EffectFactory.h"
#include "../Effects/IGhostTrailEffect.h"
#include "../Effects/GhostTrailRenderer.h"
#include <cute_draw.h>

using namespace Cute;

// Helper function to get PNG dimensions
static bool getPNGDimensions(const std::string &path, uint32_t &width, uint32_t &height)
{
    size_t file_size = 0;
    void *file_data = cf_fs_read_entire_file_to_memory(path.c_str(), &file_size);

    if (!file_data)
    {
        return false;
    }

    spng_ctx *ctx = spng_ctx_new(0);
    if (!ctx)
    {
        cf_free(file_data);
        return false;
    }

    spng_set_png_buffer(ctx, file_data, file_size);

    struct spng_ihdr ihdr;
    int ret = spng_get_ihdr(ctx, &ihdr);

    if (ret == 0)
    {
        width = ihdr.width;
        height = ihdr.height;
    }

    spng_ctx_free(ctx);
    cf_free(file_data);

    return ret == 0;
}

// Constructor
AnimatedDataCharacter::AnimatedDataCharacter()
    : initialized(false), demoTime(0.0f), directionChangeTime(0.0f), animationChangeTime(0.0f),
      currentAnimation("idle"), currentDirection(Direction::DOWN), currentFrame(0), frameTimer(0.0f),
      position(v2(0, 0)), wasMoving(false), isDoingAction(false), hitboxDebugActive(false), hitboxSize(32.0f), hitboxDistance(0.0f),
      hitboxShape(HitboxShape::SQUARE), level(nullptr), actionPointerA(0), actionPointerB(0), activeAction(nullptr), stageOfLife(StageOfLife::Alive),
      inventory(1)
{
    // Initialize input state
    for (int i = 0; i < 4; i++)
    {
        keysPressed[i] = false;
    }
    for (int i = 0; i < 2; i++)
    {
        animationKeys[i] = false;
    }
}

// Destructor
AnimatedDataCharacter::~AnimatedDataCharacter()
{
    // Cleanup character hitbox if it exists
    if (characterHitbox)
    {
        delete characterHitbox;
        characterHitbox = nullptr;
    }
} // Initialize the character with a folder path containing character.json
bool AnimatedDataCharacter::init(const std::string &folderPath)
{
    // Construct path to character.json in the folder
    std::string characterFilePath = folderPath + "/character.json";

    // Load the datafile
    if (!datafile.load(characterFilePath))
    {
        printf("AnimatedDataCharacter: ERROR: Failed to load character.json from %s\n", folderPath.c_str());
        return false;
    }

    // DataFile inherits from nlohmann::json, so use it directly as the character config
    if (!datafile.contains("name") || !datafile.contains("layers"))
    {
        printf("AnimatedDataCharacter: ERROR: character.json missing required fields (name or layers)\n");
        return false;
    }

    // Load innate actions if specified in JSON
    if (datafile.contains("innate_actions") && datafile["innate_actions"].is_array())
    {
        for (const auto &actionPath : datafile["innate_actions"])
        {
            if (actionPath.is_string())
            {
                std::string actionName = actionPath;
                // Build full path to action folder
                std::string fullPath = "/assets/DataFiles/Actions/" + actionName;
                if (addAction(fullPath))
                {
                    printf("AnimatedDataCharacter: Loaded innate action '%s' from '%s'\n", actionName.c_str(), fullPath.c_str());
                }
                else
                {
                    printf("AnimatedDataCharacter: WARNING: Failed to load innate action '%s' from '%s'\n", actionName.c_str(), fullPath.c_str());
                }
            }
        }
    }

    // Load hitbox size and distance if specified in JSON (used for actions)
    if (datafile.contains("hitbox_size") && datafile["hitbox_size"].is_number())
    {
        hitboxSize = datafile["hitbox_size"];
    }
    if (datafile.contains("hitbox_distance") && datafile["hitbox_distance"].is_number())
    {
        hitboxDistance = datafile["hitbox_distance"];
    }

    // Create default character hitbox - a single tile at the bottom of the sprite
    // This represents the character's physical footprint
    std::vector<HitboxTile> characterHitboxTiles;
    HitboxTile bottomTile;
    bottomTile.x = 0;
    bottomTile.y = 0; // At character position (bottom center)
    bottomTile.delay = 0.0f;
    bottomTile.damageModifier = 1.0f;
    characterHitboxTiles.push_back(bottomTile);

    // Create character hitbox from the single tile - this will be centered at the character's position
    characterHitbox = new HitBox();
    for (Direction direction : {Direction::UP, Direction::DOWN, Direction::LEFT, Direction::RIGHT})
    {
        characterHitbox->boxesByDirection[direction] = HitBox::buildFromTiles(characterHitboxTiles, hitboxSize, 0.0f, direction);
        characterHitbox->boundingBoxByDirection[direction] = HitBox::buildBoundingBox(characterHitbox->boxesByDirection[direction], direction);
    }

    std::string characterName = datafile["name"];
    printf("AnimatedDataCharacter: Loading character '%s' from datafile\n", characterName.c_str());

    // Get the first layer from the layers array
    if (!datafile["layers"].is_array() || datafile["layers"].empty())
    {
        printf("AnimatedDataCharacter: ERROR: layers is empty or not an array\n");
        return false;
    }

    auto &firstLayer = datafile["layers"][0];

    if (!firstLayer.contains("filename"))
    {
        printf("AnimatedDataCharacter: ERROR: First layer missing 'filename' field\n");
        return false;
    }

    if (!firstLayer.contains("tile_size"))
    {
        printf("AnimatedDataCharacter: ERROR: First layer missing 'tile_size' field\n");
        return false;
    }

    // Collect all layer filenames
    std::vector<std::string> layerFilenames;
    for (size_t i = 0; i < datafile["layers"].size(); i++)
    {
        auto &layer = datafile["layers"][i];
        if (layer.contains("filename"))
        {
            std::string filename = layer["filename"];
            layerFilenames.push_back(filename);
            printf("AnimatedDataCharacter: Added layer %zu: %s\n", i, filename.c_str());
        }
    }

    if (layerFilenames.empty())
    {
        printf("AnimatedDataCharacter: ERROR: No valid layer filenames found\n");
        return false;
    }

    std::string layerFilename = firstLayer["filename"];
    int tileSize = firstLayer["tile_size"];

    printf("AnimatedDataCharacter: Using %zu layers\n", layerFilenames.size());
    printf("AnimatedDataCharacter: Using tile size: %d\n", tileSize);

    // Construct paths using the first layer filename from the datafile for dimension checking
    std::string idle_body_path = "assets/Art/AnimationsSheets/idle/" + layerFilenames[0];
    std::string walkcycle_body_path = "assets/Art/AnimationsSheets/walkcycle/" + layerFilenames[0];

    // Get dimensions for idle animation
    uint32_t idle_width = 0, idle_height = 0;
    if (!getPNGDimensions(idle_body_path, idle_width, idle_height))
    {
        printf("AnimatedDataCharacter: ERROR: Cannot read dimensions from %s\n", idle_body_path.c_str());
        return false;
    }

    // Get dimensions for walkcycle animation
    uint32_t walkcycle_width = 0, walkcycle_height = 0;
    if (!getPNGDimensions(walkcycle_body_path, walkcycle_width, walkcycle_height))
    {
        printf("AnimatedDataCharacter: ERROR: Cannot read dimensions from %s\n", walkcycle_body_path.c_str());
        return false;
    }

    // Calculate frame counts dynamically
    // Frames per direction = image_width / tile_size
    // Number of directions = image_height / tile_size
    int idle_frames_per_direction = idle_width / tileSize;
    int idle_direction_count = idle_height / tileSize;

    int walkcycle_frames_per_direction = walkcycle_width / tileSize;
    int walkcycle_direction_count = walkcycle_height / tileSize;

    printf("AnimatedDataCharacter: Idle dimensions: %ux%u (frames: %d, directions: %d)\n",
           idle_width, idle_height, idle_frames_per_direction, idle_direction_count);
    printf("AnimatedDataCharacter: Walkcycle dimensions: %ux%u (frames: %d, directions: %d)\n",
           walkcycle_width, walkcycle_height, walkcycle_frames_per_direction, walkcycle_direction_count);

    // Define the animation layouts using computed values and all layer filenames
    std::vector<AnimationLayout> layouts = {
        AnimationLayout(
            "idle", layerFilenames, tileSize, tileSize, idle_frames_per_direction, idle_direction_count,
            {Direction::UP, Direction::LEFT, Direction::DOWN, Direction::RIGHT}),
        AnimationLayout(
            "walkcycle", layerFilenames, tileSize, tileSize, walkcycle_frames_per_direction, walkcycle_direction_count,
            {Direction::UP, Direction::LEFT, Direction::DOWN, Direction::RIGHT})};

    // Load the animation table using our new system
    animationTable = loader.loadAnimationTable("assets/Art/AnimationsSheets", layouts);

    if (animationTable.getAnimationNames().empty())
    {
        printf("AnimatedDataCharacter: Failed to load animations from skeleton assets\n");
        return false;
    }

    // Set initial state
    currentAnimation = "idle";
    setDirection(Direction::DOWN);
    currentFrame = 0;
    frameTimer = 0.0f;

    initialized = true;
    return true;
}

// Update demo state
void AnimatedDataCharacter::update(float dt, v2 moveVector)
{
    if (!initialized)
        return;

    // Update visual effects
    if (!effectQueue.empty())
    {
        auto &front = effectQueue.front();
        if (front)
        {
            front->update(dt);
            // If the effect supports ghost trails, feed it the current position.
            if (auto ghost = dynamic_cast<IGhostTrailEffect *>(front.get()))
            {
                ghost->updateSubjectPosition(position);
            }
            if (!front->isActive())
            {
                effectQueue.pop_front();
            }
        }
        else
        {
            effectQueue.pop_front();
        }
    }
    // Don't update if Dying or Dead
    if (stageOfLife == StageOfLife::Dying || stageOfLife == StageOfLife::Dead)
        return;

    // Don't allow movement if doing an action in warmup phase
    // Allow movement during cooldown
    if (isDoingAction && activeAction && !activeAction->getInCooldown())
    {
        moveVector = v2(0, 0);
    }

    // Update all actions (even inactive ones need to tick down cooldowns from global cooldown)
    for (auto &action : actionsList)
    {
        if (action.getIsActive())
        {
            action.update(dt);
        }
    }

    demoTime += dt; // Calculate movement magnitude to determine if we're moving
    float moveMagnitude = sqrt(moveVector.x * moveVector.x + moveVector.y * moveVector.y);
    bool isMoving = moveMagnitude > 0.01f; // Small threshold to avoid floating point issues

    // Update direction based on move vector
    if (isMoving)
    {
        // Determine direction based on the dominant axis
        if (cf_abs(moveVector.x) > cf_abs(moveVector.y))
        {
            // Horizontal movement is dominant
            setDirection((moveVector.x > 0) ? Direction::RIGHT : Direction::LEFT);
        }
        else
        {
            // Vertical movement is dominant
            setDirection((moveVector.y > 0) ? Direction::UP : Direction::DOWN);
        }
    }

    // Auto-switch to walkcycle when moving, idle when stopping
    if (isMoving && !wasMoving)
    {
        if (currentAnimation != "walkcycle")
        {
            currentAnimation = "walkcycle";
            currentFrame = 0;
            frameTimer = 0.0f;
        }
    }
    else if (!isMoving && wasMoving)
    {
        if (currentAnimation != "idle")
        {
            currentAnimation = "idle";
            currentFrame = 0;
            frameTimer = 0.0f;
        }
    }
    wasMoving = isMoving;

    // Handle manual animation input (allows user to override auto-switching with 1/2/SPACE keys)
    handleInput();

    // Apply movement based on move vector
    position.x += moveVector.x * dt;
    position.y += moveVector.y * dt;

    // Update animation
    updateAnimation(dt);
}

// Handle input for demo controls
void AnimatedDataCharacter::handleInput()
{
    if (!initialized)
        return;

    // Animation controls (1, 2) - manual override of auto-switching
    bool animKey1 = key_just_pressed(CF_KEY_1); // Switch to idle
    bool animKey2 = key_just_pressed(CF_KEY_2); // Switch to walkcycle

    // Additional controls
    bool rPressed = key_just_pressed(CF_KEY_R); // Reset position

    // Handle manual animation input (overrides auto-switching)
    if (animKey1)
    {
        currentAnimation = "idle";
        currentFrame = 0;
        frameTimer = 0.0f;
    }
    else if (animKey2)
    {
        currentAnimation = "walkcycle";
        currentFrame = 0;
        frameTimer = 0.0f;
    }

    // Handle R key - reset position
    if (rPressed)
    {
        position = v2(0.0f, 0.0f);
    }
}

// Update animation state
void AnimatedDataCharacter::updateAnimation(float dt)
{
    const Animation *anim = animationTable.getAnimation(currentAnimation);
    if (!anim || anim->frames.empty())
        return;

    // Update frame timer
    frameTimer += dt * 1000.0f; // Convert to milliseconds

    // Find current frame
    const AnimationFrame *currentAnimFrame = nullptr;
    for (const auto &frame : anim->frames)
    {
        if (frame.direction == currentDirection && frame.frameIndex == currentFrame)
        {
            currentAnimFrame = &frame;
            break;
        }
    }

    // If we found a frame, check if we should advance
    if (currentAnimFrame && frameTimer >= currentAnimFrame->delay)
    {
        frameTimer = 0.0f;

        // Advance to next frame - but handle idle vs animated differently
        if (currentAnimation == "idle")
        {
            // Idle animations don't advance frames - they stay at frame 0 for each direction
            currentFrame = 0;
        }
        else
        {
            // Walkcycle and other animations advance through frames within each direction
            currentFrame++;

            // For walkcycle: 9 frames per direction, so max frame index is 8
            int maxFramesPerDirection = 9;
            if (currentFrame >= maxFramesPerDirection)
            {
                currentFrame = 0;
            }
        }
    }
}

// Cycle through directions
void AnimatedDataCharacter::cycleDirection()
{
    int currentDir = static_cast<int>(currentDirection);
    currentDir = (currentDir + 1) % 4;
    setDirection(static_cast<Direction>(currentDir));
}

// Cycle through animations
void AnimatedDataCharacter::cycleAnimation()
{
    if (currentAnimation == "idle")
    {
        currentAnimation = "walkcycle";
    }
    else
    {
        currentAnimation = "idle";
    }

    currentFrame = 0;
    frameTimer = 0.0f;
}

// Render the demo
void AnimatedDataCharacter::render()
{
    if (!initialized)
        return;

    // Don't render if Dead
    if (stageOfLife == StageOfLife::Dying || stageOfLife == StageOfLife::Dead)
        return;

    // Draw ghost trail instances behind the character if active.
    GhostTrailRenderer::renderGhostsForCharacter(*this);

    beginFrontEffect();
    renderCurrentFrame();
    endFrontEffect();
    // end
    // Note: Action hitboxes are now rendered by LevelV1::renderAgentActions()
    // Only render character's default hitbox here
    if (!isDoingAction)
        renderHitbox();
}

// Render the demo at a specific position
void AnimatedDataCharacter::render(v2 renderPosition)
{
    if (!initialized)
        return;

    // Don't render if Dead or dying
    if (stageOfLife == StageOfLife::Dead || stageOfLife == StageOfLife::Dying)
        return;

    // Draw ghost trail instances at their recorded world positions
    GhostTrailRenderer::renderGhostsForCharacter(*this);

    beginFrontEffect();
    renderCurrentFrameAt(renderPosition);
    endFrontEffect();
    // Note: Action hitboxes are now rendered by LevelV1::renderAgentActions()
    // Only render character's default hitbox here
    if (!isDoingAction)
        renderHitbox();
}

// Render the current animation frame
void AnimatedDataCharacter::renderCurrentFrame()
{
    const Animation *anim = animationTable.getAnimation(currentAnimation);
    if (!anim || anim->frames.empty())
        return;

    // Find the current frame for the current direction
    const AnimationFrame *currentAnimFrame = nullptr;
    for (const auto &frame : anim->frames)
    {
        if (frame.direction == currentDirection && frame.frameIndex == currentFrame)
        {
            currentAnimFrame = &frame;
            break;
        }
    }

    if (!currentAnimFrame)
        return;

    // Render all sprite layers (bottom to top)
    if (!currentAnimFrame->spriteLayers.empty())
    {
        for (const auto &layerSprite : currentAnimFrame->spriteLayers)
        {
            if (layerSprite.w > 0 && layerSprite.h > 0)
            {
                cf_draw_sprite(&layerSprite);
            }
        }
    }
    else if (currentAnimFrame->sprite.w > 0 && currentAnimFrame->sprite.h > 0)
    {
        // Fallback to legacy single sprite
        cf_draw_sprite(&currentAnimFrame->sprite);
    }
}

// Render the current animation frame at a specific position
void AnimatedDataCharacter::renderCurrentFrameAt(v2 renderPosition)
{
    const Animation *anim = animationTable.getAnimation(currentAnimation);
    if (!anim || anim->frames.empty())
        return;

    // Find the current frame for the current direction
    const AnimationFrame *currentAnimFrame = nullptr;
    for (const auto &frame : anim->frames)
    {
        if (frame.direction == currentDirection && frame.frameIndex == currentFrame)
        {
            currentAnimFrame = &frame;
            break;
        }
    }

    if (!currentAnimFrame)
        return;

    // Apply position transformation and render all sprite layers (bottom to top)
    cf_draw_push();
    cf_draw_translate_v2(renderPosition);

    if (!currentAnimFrame->spriteLayers.empty())
    {
        for (const auto &layerSprite : currentAnimFrame->spriteLayers)
        {
            if (layerSprite.w > 0 && layerSprite.h > 0)
            {
                cf_draw_sprite(&layerSprite);
            }
        }
    }
    else if (currentAnimFrame->sprite.w > 0 && currentAnimFrame->sprite.h > 0)
    {
        // Fallback to legacy single sprite
        cf_draw_sprite(&currentAnimFrame->sprite);
    }

    cf_draw_pop();
}

// Render debug information
void AnimatedDataCharacter::renderDebugInfo()
{
    cf_draw_push_color(make_color(1.0f, 1.0f, 1.0f, 1.0f));

    // Render demo info
    v2 textPos = v2(-600, 300);

    draw_text("AnimatedDataCharacter - Skeleton Animations", textPos);

    textPos.y -= 30;
    draw_text("Controls:", textPos);

    textPos.y -= 20;
    draw_text("WASD/Arrow Keys: Change direction", textPos);

    textPos.y -= 20;
    draw_text("1: Switch to idle animation", textPos);

    textPos.y -= 20;
    draw_text("2: Switch to walkcycle animation", textPos);

    textPos.y -= 30;
    draw_text("Current State:", textPos);

    char stateText[256];

    textPos.y -= 20;
    snprintf(stateText, sizeof(stateText), "Animation: %s", currentAnimation.c_str());
    draw_text(stateText, textPos);

    textPos.y -= 20;
    const char *directionNames[] = {"UP", "LEFT", "DOWN", "RIGHT"};
    int dirIndex = static_cast<int>(currentDirection);
    const char *dirName = (dirIndex >= 0 && dirIndex < 4) ? directionNames[dirIndex] : "UNKNOWN";
    snprintf(stateText, sizeof(stateText), "Direction: %s (%d)", dirName, dirIndex);
    draw_text(stateText, textPos);

    textPos.y -= 20;
    snprintf(stateText, sizeof(stateText), "Frame: %d", currentFrame);
    draw_text(stateText, textPos);

    textPos.y -= 20;
    snprintf(stateText, sizeof(stateText), "Position: (%.1f, %.1f)", position.x, position.y);
    draw_text(stateText, textPos);

    textPos.y -= 20;
    snprintf(stateText, sizeof(stateText), "Hitbox: %s", hitboxDebugActive ? "ON" : "OFF");
    draw_text(stateText, textPos);

    cf_draw_pop_color();
}

// Check if demo is valid
bool AnimatedDataCharacter::isValid() const
{
    return initialized && !animationTable.getAnimationNames().empty();
}

// Get current position
v2 AnimatedDataCharacter::getPosition() const
{
    return position;
}

void AnimatedDataCharacter::setPosition(v2 newPosition)
{
    position = newPosition;
}

Direction AnimatedDataCharacter::getCurrentDirection() const
{
    return currentDirection;
}

void AnimatedDataCharacter::setDirection(Direction direction)
{
    currentDirection = direction;
}

void AnimatedDataCharacter::setLevel(LevelV1 *levelPtr)
{
    level = levelPtr;
}

LevelV1 *AnimatedDataCharacter::getLevel() const
{
    return level;
}

const std::string &AnimatedDataCharacter::getDataFilePath() const
{
    return datafile.getpath();
}

HitBox *AnimatedDataCharacter::getHitbox() const
{
    return characterHitbox;
}

void AnimatedDataCharacter::sethitboxDebugActive(bool active)
{
    hitboxDebugActive = active;
}

void AnimatedDataCharacter::setDoingAction(bool doing)
{
    isDoingAction = doing;

    // Clear active action when no longer doing action
    if (!doing)
    {
        // check all other actions in case one got interrupted
        // if any action is still active, set it as the active action instead
        Action *stillActiveAction = nullptr;
        for (auto &action : actionsList)
        {
            if (action.getIsActive())
            {
                stillActiveAction = &action;
                break;
            }
        }

        if (stillActiveAction)
        {
            setActiveAction(stillActiveAction);
            isDoingAction = true; // Keep doing action since we found an active one
        }
        else
        {
            setActiveAction(nullptr);
        }
    }
}

bool AnimatedDataCharacter::getIsDoingAction() const
{
    if (isDoingAction)
    {
        // if doing an action, check if not in cooldown
        if (activeAction)
        {
            if (activeAction->getInCooldown())
            {
                return false;
            }
        }
    }
    return isDoingAction;
}

void AnimatedDataCharacter::setActiveAction(Action *action)
{
    activeAction = action;
}

Action *AnimatedDataCharacter::getActiveAction() const
{
    return activeAction;
}

IGhostTrailEffect *AnimatedDataCharacter::getActiveGhostTrailEffect() const
{
    if (!effectQueue.empty() && effectQueue.front())
    {
        return dynamic_cast<IGhostTrailEffect *>(effectQueue.front().get());
    }
    return nullptr;
}

void AnimatedDataCharacter::triggerEffect(const std::string &name, int flashes, float totalDuration, float maxIntensity)
{
    auto effect = EffectFactory::makeEffect(name);
    if (effect)
    {
        effect->trigger(flashes, totalDuration, maxIntensity);
        effectQueue.push_back(std::move(effect));
    }
}

void AnimatedDataCharacter::triggerEffect(const std::string &name, int flashes, float totalDuration, float maxIntensity, std::function<void()> onComplete)
{
    auto effect = EffectFactory::makeEffect(name);
    if (effect)
    {
        effect->setOnComplete(std::move(onComplete));
        effect->trigger(flashes, totalDuration, maxIntensity);
        effectQueue.push_back(std::move(effect));
    }
}

void AnimatedDataCharacter::beginFrontEffect()
{
    if (!effectQueue.empty() && effectQueue.front())
    {
        effectQueue.front()->beginDraw();
    }
}

void AnimatedDataCharacter::endFrontEffect()
{
    if (!effectQueue.empty() && effectQueue.front())
    {
        effectQueue.front()->endDraw();
    }
}
void AnimatedDataCharacter::renderActionHitbox()
{
    // Only render if doing an action and have an active action
    // Don't render during cooldown phase
    if (isDoingAction && activeAction && !activeAction->getInCooldown())
    {
        // Calculate blended color from yellow to red based on warmup progress
        CF_Color yellow = cf_make_color_rgb(200, 200, 0);
        CF_Color red = cf_make_color_rgb(255, 0, 0);

        // Get warmup time from action JSON (in ms, convert to seconds)
        float warmupMs = activeAction->contains("warmup") ? (*activeAction)["warmup"].get<float>() : 0.0f;
        float warmupTime = warmupMs / 1000.0f;

        // Get current warmup timer
        float currentWarmupTime = activeAction->getWarmupTimer();

        CF_Color blendedColor = blend(yellow, red, warmupTime, currentWarmupTime);

        activeAction->renderHitbox(blendedColor);
    }
}

void AnimatedDataCharacter::renderHitbox()
{
    // Note: Action hitboxes are now rendered separately by LevelV1::renderAgentActions()
    // This method now only renders the character's default hitbox

    // Render character's default hitbox
    if (!hitboxDebugActive || !characterHitbox)
        return;

    // Default color is yellow for character hitbox
    CF_Color color = cf_make_color_rgb(255, 255, 0); // Yellow

    // Check if this character is inside any active action hitbox in warmup phase
    bool insideActionHitbox = false;
    if (level)
    {
        CF_Aabb characterBox = characterHitbox->getBoundingBox(currentDirection, position);
        insideActionHitbox = level->isCharacterInActionHitbox(this, characterBox);
    }

    // Set color based on state
    if (insideActionHitbox)
    {
        // Red if inside an action hitbox during warmup
        color = cf_make_color_rgb(255, 0, 0); // Red
    }
    else if (level && level->checkAgentsInArea(characterHitbox->getBoxes(currentDirection, position),
                                               characterHitbox->getBoundingBox(currentDirection, position), this))
    {
        // Orange if other agents are detected nearby
        color = cf_make_color_rgb(255, 165, 0); // Orange
    }

    // Draw all hitboxes as thick outlined rectangles
    cf_draw_push_color(color);
    cf_draw_push_antialias(false);

    for (const auto &box : characterHitbox->getBoxes(currentDirection, position))
    {
        // Draw thick outline (thickness, chubbiness/rounding)
        cf_draw_box(box, 3.0f, 0.0f);
    }

    cf_draw_pop_antialias();
    cf_draw_pop_color();
}

// Add an action to the actions list by loading from folder path
bool AnimatedDataCharacter::addAction(const std::string &folderPath)
{
    Action newAction(folderPath);

    // Check if action loaded successfully
    if (!newAction.contains("name"))
    {
        printf("AnimatedDataCharacter: Failed to add action from '%s' - no 'name' field found\n", folderPath.c_str());
        return false;
    }

    std::string actionName = newAction["name"];

    // Check if action with same name already exists
    for (const auto &existingAction : actionsList)
    {
        if (existingAction.contains("name") && existingAction["name"] == actionName)
        {
            printf("AnimatedDataCharacter: Action '%s' already exists in actions list\n", actionName.c_str());
            return false;
        }
    }
    // tell action about self
    newAction.setCharacter(this);
    // add to list
    actionsList.push_back(newAction);

    // update pointer if len of list is 1 or 2
    // update the action pointers accordingly
    if (actionsList.size() == 1)
    {
        setActionPointerA(0);
    }
    else if (actionsList.size() == 2)
    {
        setActionPointerB(1);
    }

    printf("AnimatedDataCharacter: Added action '%s' to actions list\n", actionName.c_str());
    return true;
}

// Remove an action from the actions list by name
bool AnimatedDataCharacter::removeAction(const std::string &actionName)
{
    for (auto it = actionsList.begin(); it != actionsList.end(); ++it)
    {
        if (it->contains("name") && (*it)["name"] == actionName)
        {
            actionsList.erase(it);
            printf("AnimatedDataCharacter: Removed action '%s' from actions list\n", actionName.c_str());
            return true;
        }
    }

    printf("AnimatedDataCharacter: Action '%s' not found in actions list\n", actionName.c_str());
    return false;
}

// Get the actions list
const std::vector<Action> &AnimatedDataCharacter::getActions() const
{
    return actionsList;
}

// Set action pointer A to a specific index
void AnimatedDataCharacter::setActionPointerA(size_t index)
{
    if (index < actionsList.size())
    {
        actionPointerA = index;
        printf("AnimatedDataCharacter: Action pointer A set to index %zu\n", index);
    }
    else
    {
        printf("AnimatedDataCharacter: Warning - index %zu out of bounds for action pointer A (size: %zu)\n", index, actionsList.size());
    }
}

// Set action pointer B to a specific index
void AnimatedDataCharacter::setActionPointerB(size_t index)
{
    if (index < actionsList.size())
    {
        actionPointerB = index;
        printf("AnimatedDataCharacter: Action pointer B set to index %zu\n", index);
    }
    else
    {
        printf("AnimatedDataCharacter: Warning - index %zu out of bounds for action pointer B (size: %zu)\n", index, actionsList.size());
    }
}

// Get action pointer A
Action *AnimatedDataCharacter::getActionPointerA() const
{
    if (actionPointerA < actionsList.size())
    {
        return const_cast<Action *>(&actionsList[actionPointerA]);
    }
    return nullptr;
}

// Get action pointer B
Action *AnimatedDataCharacter::getActionPointerB() const
{
    if (actionPointerB < actionsList.size())
    {
        return const_cast<Action *>(&actionsList[actionPointerB]);
    }
    return nullptr;
}

void AnimatedDataCharacter::applyGlobalCooldown(float globalCooldownSeconds)
{
    // Apply the global cooldown to all actions
    for (auto &action : actionsList)
    {
        action.applyCooldown(globalCooldownSeconds);
    }
}

// Handle hit from another character
void AnimatedDataCharacter::OnHit(AnimatedDataCharacter *character, Damage damage)
{
    // Trigger red flash effect when hit
    triggerEffect("red", 3, 1.0f, 0.80f, [this]()
                  { this->setStageOfLife(StageOfLife::Dying); });

    // Print debug message
    printf("AnimatedDataCharacter: Hit by character with damage value: %.2f\n", damage.value);
    // TODO call damage.DoDamage(this);
    // Implement health system, reactions, etc.

    // For demo purposes, set stage of life to Dying
    // setStageOfLife(StageOfLife::Dying);
}

// Set stage of life
void AnimatedDataCharacter::setStageOfLife(StageOfLife stage)
{
    // If setting to Dead, ensure we were Dying first
    if (stage == StageOfLife::Dead && stageOfLife != StageOfLife::Dying)
    {
        // todo handle this better
        printf("AnimatedDataCharacter: WARNING: Setting stage to Dead without being Dying first (current stage: %d)\n", static_cast<int>(stageOfLife));
    }

    stageOfLife = stage;
}

// Get stage of life
StageOfLife AnimatedDataCharacter::getStageOfLife() const
{
    return stageOfLife;
}

// Inventory access
Inventory &AnimatedDataCharacter::getInventory()
{
    return inventory;
}

const Inventory &AnimatedDataCharacter::getInventory() const
{
    return inventory;
}