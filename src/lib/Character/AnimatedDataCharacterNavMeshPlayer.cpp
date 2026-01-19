#include "AnimatedDataCharacterNavMeshPlayer.h"
#include <cute.h>

using namespace Cute;

// Constructor
AnimatedDataCharacterNavMeshPlayer::AnimatedDataCharacterNavMeshPlayer()
    : AnimatedDataCharacter(), navmesh(nullptr), currentPolygon(-1), spriteWidth(64.0f), spriteHeight(64.0f), abActions(nullptr)
{
}

// Destructor
AnimatedDataCharacterNavMeshPlayer::~AnimatedDataCharacterNavMeshPlayer()
{
    // We don't own the navmesh, so we don't delete it
}

// Set the navmesh this player is operating on
void AnimatedDataCharacterNavMeshPlayer::setNavMesh(NavMesh *navmesh)
{
    this->navmesh = navmesh;
    updateCurrentPolygon();
}

// Get the navmesh this player is on
NavMesh *AnimatedDataCharacterNavMeshPlayer::getNavMesh() const
{
    return navmesh;
}

// Check if player has a navmesh assigned
bool AnimatedDataCharacterNavMeshPlayer::hasNavMesh() const
{
    return navmesh != nullptr;
}

// Get the current polygon the player is on
int AnimatedDataCharacterNavMeshPlayer::getCurrentPolygon() const
{
    return currentPolygon;
}

// Set the sprite dimensions (used to calculate collision box position)
void AnimatedDataCharacterNavMeshPlayer::setSpriteDimensions(float width, float height)
{
    spriteWidth = width;
    spriteHeight = height;
}

// Update the player's current polygon based on position
void AnimatedDataCharacterNavMeshPlayer::updateCurrentPolygon()
{
    if (!navmesh)
    {
        currentPolygon = -1;
        return;
    }

    // Use the center bottom of the collision box for polygon detection
    CF_Aabb collisionBox = getNavMeshCollisionBox();
    float centerX = (collisionBox.min.x + collisionBox.max.x) / 2.0f;
    float centerY = ((collisionBox.min.y + collisionBox.max.y) / 2.0f) + (COLLISION_BOX_HEIGHT / 2.0f);
    CF_V2 checkPosition = cf_v2(centerX, centerY);

    currentPolygon = navmesh->findPolygonAt(checkPosition);
}

// Check if player is currently on walkable area
bool AnimatedDataCharacterNavMeshPlayer::isOnWalkableArea() const
{
    if (!navmesh)
    {
        return false;
    }

    // Use the center bottom of the collision box for walkable check
    CF_Aabb collisionBox = getNavMeshCollisionBox();
    float centerX = (collisionBox.min.x + collisionBox.max.x) / 2.0f;
    float centerY = ((collisionBox.min.y + collisionBox.max.y) / 2.0f) + (COLLISION_BOX_HEIGHT / 2.0f);
    CF_V2 checkPosition = cf_v2(centerX, centerY);

    return navmesh->isWalkable(checkPosition);
}

// Get the navmesh collision box (thin horizontal box at player's feet)
CF_Aabb AnimatedDataCharacterNavMeshPlayer::getNavMeshCollisionBox() const
{
    v2 pos = getPosition();

    // Collision box is a horizontal line at the very bottom of the sprite
    // In Cute Framework, Y increases upward, so bottom of sprite is at pos.y - spriteHeight/2
    // We want the box's bottom edge to align with the sprite's bottom edge
    float spriteBottomY = pos.y - spriteHeight;

    // Width spans the sprite width, centered horizontally
    float halfWidth = spriteWidth / 2.0f;

    // Create a thin horizontal box at the feet (min.y is bottom, max.y is top)
    return cf_make_aabb(
        cf_v2(pos.x - halfWidth, spriteBottomY),
        cf_v2(pos.x + halfWidth, spriteBottomY + COLLISION_BOX_HEIGHT));
}

// Debug render the navmesh collision box
void AnimatedDataCharacterNavMeshPlayer::debugRenderNavMeshCollisionBox() const
{
    CF_Aabb box = getNavMeshCollisionBox();

    // Draw as a bright magenta/pink line for visibility
    cf_draw_push_color(cf_make_color_rgb(255, 0, 255));
    cf_draw_quad(box, 0.0f, 2.0f); // 2px thick outline
    cf_draw_pop_color();
}

// Check if a position would be on walkable area
bool AnimatedDataCharacterNavMeshPlayer::wouldBeOnWalkableArea(v2 futurePosition) const
{
    if (!navmesh)
    {
        return true; // No navmesh means no restrictions
    }

    // Get current collision box center
    CF_Aabb currentBox = getNavMeshCollisionBox();
    float currentCenterX = (currentBox.min.x + currentBox.max.x) / 2.0f;
    float currentCenterY = ((currentBox.min.y + currentBox.max.y) / 2.0f) + (COLLISION_BOX_HEIGHT / 2.0f);
    CF_V2 currentPosition = cf_v2(currentCenterX, currentCenterY);

    // Calculate what the collision box would be at the future position
    float spriteBottomY = futurePosition.y - spriteHeight;
    float halfWidth = spriteWidth / 2.0f;

    CF_Aabb futureBox = cf_make_aabb(
        cf_v2(futurePosition.x - halfWidth, spriteBottomY),
        cf_v2(futurePosition.x + halfWidth, spriteBottomY + COLLISION_BOX_HEIGHT));

    // Use the center bottom of the collision box for walkable check
    float centerX = (futureBox.min.x + futureBox.max.x) / 2.0f;
    float centerY = ((futureBox.min.y + futureBox.max.y) / 2.0f) + (COLLISION_BOX_HEIGHT / 2.0f);
    CF_V2 checkPosition = cf_v2(centerX, centerY);

    // Check if the future position is on walkable area
    if (!navmesh->isWalkable(checkPosition))
    {
        return false;
    }

    // Check if the movement path crosses any boundary edges (including cuts)
    if (navmesh->crossesBoundaryEdge(currentPosition, checkPosition))
    {
        return false;
    }

    return true;
}

// Update with navmesh collision checking
void AnimatedDataCharacterNavMeshPlayer::update(float dt, v2 moveVector)
{
    // If we have a navmesh, check if the move would take us off walkable area
    if (navmesh)
    {
        // Calculate where we would end up with this move
        v2 currentPos = getPosition();
        v2 futurePos = cf_v2(
            currentPos.x + moveVector.x * dt,
            currentPos.y + moveVector.y * dt);

        // Check if that position would be on walkable area
        if (!wouldBeOnWalkableArea(futurePos))
        {
            // Can't move there - call parent update with zero move vector
            // This will switch animation to idle and not move the player
            AnimatedDataCharacter::update(dt, cf_v2(0.0f, 0.0f));
            updateCurrentPolygon();
            return;
        }
    }

    // Move is valid, call parent update normally
    AnimatedDataCharacter::update(dt, moveVector);

    // Update our current polygon after movement
    if (navmesh)
    {
        updateCurrentPolygon();
    }
}

// Set ABActions
void AnimatedDataCharacterNavMeshPlayer::setABActions(ABActions *actions)
{
    abActions = actions;
}

// Get ABActions
ABActions *AnimatedDataCharacterNavMeshPlayer::getABActions() const
{
    return abActions;
}

// Calculate ABActions (pass-through)
void AnimatedDataCharacterNavMeshPlayer::calculateABActions()
{
    // remake ABActions class
    abActions = new ABActions();
    Action *actionA = getActionPointerA();
    Action *actionB = getActionPointerB();
    abActions->setActionA(actionA);
    abActions->setActionB(actionB);
    abActions->calculate();
}

// Override setActionPointerA to recalculate ABActions
void AnimatedDataCharacterNavMeshPlayer::setActionPointerA(size_t index)
{
    AnimatedDataCharacter::setActionPointerA(index);
    calculateABActions();
}

// Override setActionPointerB to recalculate ABActions
void AnimatedDataCharacterNavMeshPlayer::setActionPointerB(size_t index)
{
    AnimatedDataCharacter::setActionPointerB(index);
    calculateABActions();
}

// Move action pointer A up (increment with wrapping)
void AnimatedDataCharacterNavMeshPlayer::MoveActionPointerAUp()
{
    const auto &actions = getActions();
    if (actions.empty())
        return;

    Action *currentPointer = getActionPointerA();
    if (!currentPointer)
    {
        setActionPointerA(0);
        return;
    }

    // Find current index
    size_t currentIndex = 0;
    for (size_t i = 0; i < actions.size(); ++i)
    {
        if (&actions[i] == currentPointer)
        {
            currentIndex = i;
            break;
        }
    }

    // Increment with wrapping
    size_t newIndex = (currentIndex + 1) % actions.size();
    setActionPointerA(newIndex);
}

// Move action pointer A down (decrement with wrapping)
void AnimatedDataCharacterNavMeshPlayer::MoveActionPointerADown()
{
    const auto &actions = getActions();
    if (actions.empty())
        return;

    Action *currentPointer = getActionPointerA();
    if (!currentPointer)
    {
        setActionPointerA(0);
        return;
    }

    // Find current index
    size_t currentIndex = 0;
    for (size_t i = 0; i < actions.size(); ++i)
    {
        if (&actions[i] == currentPointer)
        {
            currentIndex = i;
            break;
        }
    }

    // Decrement with wrapping
    size_t newIndex = (currentIndex == 0) ? actions.size() - 1 : currentIndex - 1;
    setActionPointerA(newIndex);
}

// Move action pointer B up (increment with wrapping)
void AnimatedDataCharacterNavMeshPlayer::MoveActionPointerBUp()
{
    const auto &actions = getActions();
    if (actions.empty())
        return;

    Action *currentPointer = getActionPointerB();
    if (!currentPointer)
    {
        setActionPointerB(0);
        return;
    }

    // Find current index
    size_t currentIndex = 0;
    for (size_t i = 0; i < actions.size(); ++i)
    {
        if (&actions[i] == currentPointer)
        {
            currentIndex = i;
            break;
        }
    }

    // Increment with wrapping
    size_t newIndex = (currentIndex + 1) % actions.size();
    setActionPointerB(newIndex);
}

// Move action pointer B down (decrement with wrapping)
void AnimatedDataCharacterNavMeshPlayer::MoveActionPointerBDown()
{
    const auto &actions = getActions();
    if (actions.empty())
        return;

    Action *currentPointer = getActionPointerB();
    if (!currentPointer)
    {
        setActionPointerB(0);
        return;
    }

    // Find current index
    size_t currentIndex = 0;
    for (size_t i = 0; i < actions.size(); ++i)
    {
        if (&actions[i] == currentPointer)
        {
            currentIndex = i;
            break;
        }
    }

    // Decrement with wrapping
    size_t newIndex = (currentIndex == 0) ? actions.size() - 1 : currentIndex - 1;
    setActionPointerB(newIndex);
}
