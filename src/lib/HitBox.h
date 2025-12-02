#ifndef HITBOX_H
#define HITBOX_H

#include "lib/SpriteAnimationLoader.h"

enum class HitboxShape
{
    SQUARE,
    T_SHAPE,
    L_SHAPE
};

class HitBox
{
public:
    HitBox();
    ~HitBox();

    static HitBox* createHitBox(HitboxShape shape, float hitboxSize, float hitboxDistance);
    std::vector<CF_Aabb> getBoxes(Direction direction, v2 translation);
    CF_Aabb getBoundingBox(Direction direction, v2 translation);

    std::map<Direction, std::vector<CF_Aabb>> boxesByDirection;
    std::map<Direction, CF_Aabb> boundingBoxByDirection;

    static std::vector<CF_Aabb> buildTShape(float hitboxSize, float hitboxDistance, Direction direction);
    static std::vector<CF_Aabb> buildLShape(float hitboxSize, float hitboxDistance, Direction direction);
    static std::vector<CF_Aabb> buildSquare(float hitboxSize, float hitboxDistance, Direction direction);
    static CF_Aabb buildBoundingBox(std::vector<CF_Aabb> boxes, Direction direction);
};

#endif // HITBOX_H