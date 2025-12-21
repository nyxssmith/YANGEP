#ifndef HITBOX_H
#define HITBOX_H

#include "SpriteAnimationLoader.h"
#include "DataFile.h"
#include <vector>

enum class HitboxShape
{
    SQUARE,
    T_SHAPE,
    L_SHAPE,
    CUSTOM // For JSON-defined shapes
};

struct HitboxTile
{
    int x;
    int y;
    float delay;
    float damageModifier;
};

class HitBox
{
public:
    HitBox();
    ~HitBox();

    static HitBox *createHitBox(HitboxShape shape, float hitboxSize, float hitboxDistance);
    static HitBox *createHitBoxFromJson(const DataFile &hitboxData, float hitboxSize, float hitboxDistance);
    std::vector<CF_Aabb> getBoxes(Direction direction, v2 translation);
    CF_Aabb getBoundingBox(Direction direction, v2 translation);
    const std::vector<HitboxTile> &getTiles() const;
    void render(v2 characterPosition, Direction facingDirection, const class LevelV1 &level);

    std::map<Direction, std::vector<CF_Aabb>> boxesByDirection;
    std::map<Direction, CF_Aabb> boundingBoxByDirection;

    static std::vector<CF_Aabb> buildTShape(float hitboxSize, float hitboxDistance, Direction direction);
    static std::vector<CF_Aabb> buildLShape(float hitboxSize, float hitboxDistance, Direction direction);
    static std::vector<CF_Aabb> buildSquare(float hitboxSize, float hitboxDistance, Direction direction);
    static std::vector<CF_Aabb> buildFromTiles(const std::vector<HitboxTile> &tiles, float hitboxSize, float hitboxDistance, Direction direction);
    static CF_Aabb buildBoundingBox(std::vector<CF_Aabb> boxes, Direction direction);

private:
    std::vector<HitboxTile> tiles;
    static v2 rotateCoordinate(int x, int y, Direction direction);
};

#endif // HITBOX_H