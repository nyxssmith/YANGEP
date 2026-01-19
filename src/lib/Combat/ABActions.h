#pragma once

#include <cute.h>
#include <map>
#include <vector>
#include "SpriteAnimationLoader.h"

using namespace Cute;

// Forward declaration
class Action;

class ABActions
{
private:
    Action *actionA;
    Action *actionB;

public:
    ABActions();
    ~ABActions();

    // Setters
    void setActionA(Action *action);
    void setActionB(Action *action);

    // Getters
    Action *getActionA() const;
    Action *getActionB() const;

    // Calculate function - pre-calculates hitboxes for all directions
    void calculate();

    // Render pre-calculated hitboxes at a specific position and direction
    void render(v2 position, Direction direction, CF_Color colorA, CF_Color colorB,
                float border_opacity = 0.9f, float fill_opacity = 0.4f);

private:
    // Pre-calculated hitboxes for each direction
    std::map<Direction, std::vector<CF_Aabb>> actionAOnlyBoxes;
    std::map<Direction, std::vector<CF_Aabb>> actionBOnlyBoxes;
    std::map<Direction, std::vector<CF_Aabb>> bothActionBoxes;
};
