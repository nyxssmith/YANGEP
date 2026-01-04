#include "HitBox.h"
#include "SpriteAnimationLoader.h"
#include "LevelV1.h"
#include "AnimatedDataCharacter.h"
#include "../UI/HighlightTile.h"
#include <cstdio>

HitBox::HitBox()
{
}

HitBox::~HitBox()
{
}

HitBox *HitBox::createHitBoxFromJson(const DataFile &hitboxData, float hitboxSize, float hitboxDistance)
{
    HitBox *hitBox = new HitBox();

    // Parse tiles from JSON
    if (!hitboxData.contains("tiles") || !hitboxData["tiles"].is_array())
    {
        printf("HitBox: Error - hitbox.json missing 'tiles' array\n");
        delete hitBox;
        return nullptr;
    }

    for (const auto &tileJson : hitboxData["tiles"])
    {
        HitboxTile tile;
        tile.x = tileJson.value("x", 0);
        tile.y = tileJson.value("y", 0);
        tile.delay = tileJson.value("delay", 0.0f);
        tile.damageModifier = tileJson.value("damage_modifier", 1.0f);
        hitBox->tiles.push_back(tile);
    }

    // Build boxes for each direction
    for (Direction direction : {Direction::UP, Direction::DOWN, Direction::LEFT, Direction::RIGHT})
    {
        hitBox->boxesByDirection[direction] = HitBox::buildFromTiles(hitBox->tiles, hitboxSize, hitboxDistance, direction);
        hitBox->boundingBoxByDirection[direction] = HitBox::buildBoundingBox(hitBox->boxesByDirection[direction], direction);
    }

    printf("HitBox: Created custom hitbox from JSON with %zu tiles\n", hitBox->tiles.size());
    return hitBox;
}

const std::vector<HitboxTile> &HitBox::getTiles() const
{
    return tiles;
}

HitBox *HitBox::createHitBox(HitboxShape shape, float hitboxSize, float hitboxDistance)
{
    HitBox *hitBox = new HitBox();
    switch (shape)
    {
    case HitboxShape::T_SHAPE:
        for (Direction direction : {Direction::UP, Direction::DOWN, Direction::LEFT, Direction::RIGHT})
        {
            hitBox->boxesByDirection[direction] = HitBox::buildTShape(hitboxSize, hitboxDistance, direction);
            hitBox->boundingBoxByDirection[direction] = HitBox::buildBoundingBox(hitBox->boxesByDirection[direction], direction);
        }
        break;
    case HitboxShape::L_SHAPE:
        for (Direction direction : {Direction::UP, Direction::DOWN, Direction::LEFT, Direction::RIGHT})
        {
            hitBox->boxesByDirection[direction] = HitBox::buildLShape(hitboxSize, hitboxDistance, direction);
            hitBox->boundingBoxByDirection[direction] = HitBox::buildBoundingBox(hitBox->boxesByDirection[direction], direction);
        }
        break;
    case HitboxShape::SQUARE:
        for (Direction direction : {Direction::UP, Direction::DOWN, Direction::LEFT, Direction::RIGHT})
        {
            hitBox->boxesByDirection[direction] = HitBox::buildSquare(hitboxSize, hitboxDistance, direction);
            hitBox->boundingBoxByDirection[direction] = HitBox::buildBoundingBox(hitBox->boxesByDirection[direction], direction);
        }
        break;
    case HitboxShape::CUSTOM:
        // CUSTOM shapes are built via createHitBoxFromJson. Leave empty here by design.
        break;
    default:
        break;
    }
    return hitBox;
}

std::vector<CF_Aabb> HitBox::getBoxes(Direction direction, v2 translation)
{
    std::vector<CF_Aabb> boxes = boxesByDirection[direction];
    for (auto &box : boxes)
    {
        box.min += translation;
        box.max += translation;
    }
    return boxes;
}

CF_Aabb HitBox::getBoundingBox(Direction direction, v2 translation)
{
    CF_Aabb boundingBox = boundingBoxByDirection[direction];
    boundingBox.min += translation;
    boundingBox.max += translation;
    return boundingBox;
}

// Documentation:
// This function builds a bounding box for a given direction, by iterating over the boxes and finding the smallest and largest x and y values
// This is used to get the bounding box for a given direction, which is used to check if an agent is in the hitbox

CF_Aabb HitBox::buildBoundingBox(std::vector<CF_Aabb> boxes, Direction direction)
{
    // build the bounding box for the direction, by iterating over the boxes and finding the smallest and largest x and y values
    float minX = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::min();
    float minY = std::numeric_limits<float>::max();
    float maxY = std::numeric_limits<float>::min();
    for (const auto &box : boxes)
    {
        minX = std::min(minX, box.min.x);
        maxX = std::max(maxX, box.max.x);
        minY = std::min(minY, box.min.y);
        maxY = std::max(maxY, box.max.y);
    }
    return cf_make_aabb(cf_v2(minX, minY), cf_v2(maxX, maxY));
}

// builds boxes for a T shape, for a given direction, returns a vector of AABBs
std::vector<CF_Aabb> HitBox::buildTShape(float hitboxSize, float hitboxDistance, Direction direction)
{
    std::vector<CF_Aabb> hitboxes;
    float halfSize = hitboxSize / 2.0f;

    // Center box in the facing direction (stem of the T)
    v2 centerBox = cf_v2(0, 0);

    // Three boxes perpendicular to facing direction (top of the T)
    v2 leftBox, middleBox, rightBox;

    switch (direction)
    {
    case Direction::UP:
    case Direction::DOWN:
    {
        // Stem extends vertically
        centerBox.y += (direction == Direction::UP ? hitboxDistance : -hitboxDistance);

        // Top extends horizontally, positioned adjacent to the stem
        // The top row should be at the far edge of the center box, then one box further out
        float topY = centerBox.y + (direction == Direction::UP ? hitboxSize : -hitboxSize);
        leftBox = cf_v2(centerBox.x - hitboxSize, topY);
        middleBox = cf_v2(centerBox.x, topY);
        rightBox = cf_v2(centerBox.x + hitboxSize, topY);
        break;
    }

    case Direction::LEFT:
    case Direction::RIGHT:
    {
        // Stem extends horizontally
        centerBox.x += (direction == Direction::RIGHT ? hitboxDistance : -hitboxDistance);

        // Top extends vertically, positioned adjacent to the stem
        float topX = centerBox.x + (direction == Direction::RIGHT ? hitboxSize : -hitboxSize);
        leftBox = cf_v2(topX, centerBox.y + hitboxSize);
        middleBox = cf_v2(topX, centerBox.y);
        rightBox = cf_v2(topX, centerBox.y - hitboxSize);
        break;
    }
    }

    // Create AABBs for all four boxes
    hitboxes.push_back(cf_make_aabb(
        cf_v2(centerBox.x - halfSize, centerBox.y - halfSize),
        cf_v2(centerBox.x + halfSize, centerBox.y + halfSize)));

    hitboxes.push_back(cf_make_aabb(
        cf_v2(leftBox.x - halfSize, leftBox.y - halfSize),
        cf_v2(leftBox.x + halfSize, leftBox.y + halfSize)));

    hitboxes.push_back(cf_make_aabb(
        cf_v2(middleBox.x - halfSize, middleBox.y - halfSize),
        cf_v2(middleBox.x + halfSize, middleBox.y + halfSize)));

    hitboxes.push_back(cf_make_aabb(
        cf_v2(rightBox.x - halfSize, rightBox.y - halfSize),
        cf_v2(rightBox.x + halfSize, rightBox.y + halfSize)));

    return hitboxes;
}

std::vector<CF_Aabb> HitBox::buildLShape(float hitboxSize, float hitboxDistance, Direction direction)
{
    std::vector<CF_Aabb> hitboxes;
    float halfSize = hitboxSize / 2.0f;

    // Two boxes extending in facing direction (long part of L)
    v2 box1 = cf_v2(0, 0);
    v2 box2, box3, box4;

    switch (direction)
    {
    case Direction::UP:
        // Extend upward
        box1.y += hitboxDistance;
        box2 = cf_v2(box1.x, box1.y + hitboxSize);
        // Turn right at the end
        box3 = cf_v2(box1.x + hitboxSize, box2.y);
        box4 = cf_v2(box1.x + hitboxSize * 2, box2.y);
        break;

    case Direction::DOWN:
        // Extend downward
        box1.y -= hitboxDistance;
        box2 = cf_v2(box1.x, box1.y - hitboxSize);
        // Turn right at the end
        box3 = cf_v2(box1.x + hitboxSize, box2.y);
        box4 = cf_v2(box1.x + hitboxSize * 2, box2.y);
        break;

    case Direction::LEFT:
        // Extend left
        box1.x -= hitboxDistance;
        box2 = cf_v2(box1.x - hitboxSize, box1.y);
        // Turn up at the end
        box3 = cf_v2(box2.x, box1.y + hitboxSize);
        box4 = cf_v2(box2.x, box1.y + hitboxSize * 2);
        break;

    case Direction::RIGHT:
        // Extend right
        box1.x += hitboxDistance;
        box2 = cf_v2(box1.x + hitboxSize, box1.y);
        // Turn up at the end
        box3 = cf_v2(box2.x, box1.y + hitboxSize);
        box4 = cf_v2(box2.x, box1.y + hitboxSize * 2);
        break;
    }

    // Create AABBs for all four boxes
    hitboxes.push_back(cf_make_aabb(
        cf_v2(box1.x - halfSize, box1.y - halfSize),
        cf_v2(box1.x + halfSize, box1.y + halfSize)));

    hitboxes.push_back(cf_make_aabb(
        cf_v2(box2.x - halfSize, box2.y - halfSize),
        cf_v2(box2.x + halfSize, box2.y + halfSize)));

    hitboxes.push_back(cf_make_aabb(
        cf_v2(box3.x - halfSize, box3.y - halfSize),
        cf_v2(box3.x + halfSize, box3.y + halfSize)));

    hitboxes.push_back(cf_make_aabb(
        cf_v2(box4.x - halfSize, box4.y - halfSize),
        cf_v2(box4.x + halfSize, box4.y + halfSize)));

    return hitboxes;
}

// build square, just one box, for a given direction, returns a vector of AABBs, but only one box
std::vector<CF_Aabb> HitBox::buildSquare(float hitboxSize, float hitboxDistance, Direction direction)
{
    std::vector<CF_Aabb> hitboxes;
    float halfSize = hitboxSize / 2.0f;

    v2 centerBox = cf_v2(0, 0);

    switch (direction)
    {
    case Direction::UP:
        centerBox.y += hitboxDistance;
        break;
    case Direction::DOWN:
        centerBox.y -= hitboxDistance;
        break;
    case Direction::LEFT:
        centerBox.x -= hitboxDistance;
        break;
    case Direction::RIGHT:
        centerBox.x += hitboxDistance;
        break;
    }

    // Create AABBs
    hitboxes.push_back(cf_make_aabb(cf_v2(centerBox.x - halfSize, centerBox.y - halfSize), cf_v2(centerBox.x + halfSize, centerBox.y + halfSize)));

    return hitboxes;
}

// Rotate coordinates based on direction (default is RIGHT)
v2 HitBox::rotateCoordinate(int x, int y, Direction direction)
{
    // Input coordinates assume facing RIGHT
    // x is horizontal distance (positive = right)
    // y is perpendicular distance (positive = away from character)

    switch (direction)
    {
    case Direction::RIGHT:
        // No rotation needed - this is the default
        return cf_v2((float)x, (float)y);

    case Direction::UP:
        // Rotate 90 degrees counter-clockwise
        // Right's y-axis becomes Up's x-axis, Right's x-axis becomes Up's y-axis
        return cf_v2((float)y, (float)x);

    case Direction::LEFT:
        // Rotate 180 degrees
        // Right becomes Left, so x becomes -x, y becomes -y
        return cf_v2((float)-x, (float)-y);

    case Direction::DOWN:
        // Rotate 90 degrees clockwise
        // Right's y-axis becomes Down's -x-axis, Right's x-axis becomes Down's -y-axis
        return cf_v2((float)-y, (float)-x);
    }

    return cf_v2((float)x, (float)y);
}

// Build hitbox from tile definitions in JSON
std::vector<CF_Aabb> HitBox::buildFromTiles(const std::vector<HitboxTile> &tiles, float hitboxSize, float hitboxDistance, Direction direction)
{
    std::vector<CF_Aabb> hitboxes;
    float halfSize = hitboxSize / 2.0f;

    for (const auto &tile : tiles)
    {
        // Rotate the tile coordinates based on direction
        v2 rotated = rotateCoordinate(tile.x, tile.y, direction);

        // Scale by hitboxSize and add distance offset
        v2 tileCenter = cf_v2(
            rotated.x * hitboxSize,
            rotated.y * hitboxSize);

        // Add the base distance in the facing direction
        switch (direction)
        {
        case Direction::UP:
            tileCenter.y += hitboxDistance;
            break;
        case Direction::DOWN:
            tileCenter.y -= hitboxDistance;
            break;
        case Direction::LEFT:
            tileCenter.x -= hitboxDistance;
            break;
        case Direction::RIGHT:
            tileCenter.x += hitboxDistance;
            break;
        }

        // Create AABB for this tile
        hitboxes.push_back(cf_make_aabb(
            cf_v2(tileCenter.x - halfSize, tileCenter.y - halfSize),
            cf_v2(tileCenter.x + halfSize, tileCenter.y + halfSize)));
    }

    return hitboxes;
}

// Render hitbox by highlighting the actual bounding boxes
void HitBox::render(v2 characterPosition, Direction facingDirection, const LevelV1 &level, CF_Color color)
{
    // Get hitbox boxes for the facing direction at character position
    std::vector<CF_Aabb> boxes = getBoxes(facingDirection, characterPosition);

    // Highlight each box directly (not snapped to grid)
    for (const auto &box : boxes)
    {
        highlightArea(box, color, 0.9f, 0.4f);
    }
}