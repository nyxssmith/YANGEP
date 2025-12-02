#include "HitBox.h"
#include "lib/SpriteAnimationLoader.h"

HitBox::HitBox()
{
}

HitBox::~HitBox()
{
}

HitBox* HitBox::createHitBox(HitboxShape shape, float hitboxSize, float hitboxDistance)
{
    HitBox* hitBox = new HitBox();
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
    }
    return hitBox;
}

std::vector<CF_Aabb> HitBox::getBoxes(Direction direction, v2 translation)
{
    std::vector<CF_Aabb> boxes = boxesByDirection[direction];
    for (auto& box : boxes)
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
    for (const auto& box : boxes)
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
        cf_v2(centerBox.x + halfSize, centerBox.y + halfSize)
    ));

    hitboxes.push_back(cf_make_aabb(
        cf_v2(leftBox.x - halfSize, leftBox.y - halfSize),
        cf_v2(leftBox.x + halfSize, leftBox.y + halfSize)
    ));

    hitboxes.push_back(cf_make_aabb(
        cf_v2(middleBox.x - halfSize, middleBox.y - halfSize),
        cf_v2(middleBox.x + halfSize, middleBox.y + halfSize)
    ));

    hitboxes.push_back(cf_make_aabb(
        cf_v2(rightBox.x - halfSize, rightBox.y - halfSize),
        cf_v2(rightBox.x + halfSize, rightBox.y + halfSize)
    ));

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
        cf_v2(box1.x + halfSize, box1.y + halfSize)
    ));

    hitboxes.push_back(cf_make_aabb(
        cf_v2(box2.x - halfSize, box2.y - halfSize),
        cf_v2(box2.x + halfSize, box2.y + halfSize)
    ));

    hitboxes.push_back(cf_make_aabb(
        cf_v2(box3.x - halfSize, box3.y - halfSize),
        cf_v2(box3.x + halfSize, box3.y + halfSize)
    ));

    hitboxes.push_back(cf_make_aabb(
        cf_v2(box4.x - halfSize, box4.y - halfSize),
        cf_v2(box4.x + halfSize, box4.y + halfSize)
    ));

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