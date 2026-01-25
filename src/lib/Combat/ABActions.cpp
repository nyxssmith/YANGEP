#include "ABActions.h"
#include "Action.h"
#include "HitBox.h"
#include "HighlightTile.h"
#include <cute.h>

ABActions::ABActions() : actionA(nullptr), actionB(nullptr)
{
}

ABActions::~ABActions()
{
    // Note: We don't delete the actions as we don't own them
}

void ABActions::setActionA(Action *action)
{
    actionA = action;
}

void ABActions::setActionB(Action *action)
{
    actionB = action;
}

Action *ABActions::getActionA() const
{
    return actionA;
}

Action *ABActions::getActionB() const
{
    return actionB;
}

void ABActions::calculate()
{
    printf("ABActions: Calculating with Action A: %s, Action B: %s\n",
           actionA ? actionA->contains("name") ? (*actionA)["name"].get<std::string>().c_str() : "unnamed" : "null",
           actionB ? actionB->contains("name") ? (*actionB)["name"].get<std::string>().c_str() : "unnamed" : "null");

    // Clear previous calculations
    actionAOnlyBoxes.clear();
    actionBOnlyBoxes.clear();
    bothActionBoxes.clear();

    // Can't calculate if both actions are null
    if (!actionA && !actionB)
    {
        printf("ABActions: Cannot calculate - both actions are null\n");
        return;
    }

    // If only action A exists
    if (actionA && !actionB)
    {
        HitBox *hitboxA = actionA->getHitBox();
        if (!hitboxA)
        {
            printf("ABActions: Action A has no hitbox\n");
            return;
        }

        const std::vector<HitboxTile> &tilesA = hitboxA->getTiles();

        for (Direction dir : {Direction::UP, Direction::DOWN, Direction::LEFT, Direction::RIGHT})
        {
            std::vector<CF_Aabb> tempBoxesA = hitboxA->getBoxes(dir, cf_v2(0, 0));
            float hitboxSizeA = tempBoxesA.empty() ? 32.0f : (tempBoxesA[0].max.x - tempBoxesA[0].min.x);

            for (const auto &tile : tilesA)
            {
                std::vector<HitboxTile> singleTile = {tile};
                std::vector<CF_Aabb> boxes = HitBox::buildFromTiles(singleTile, hitboxSizeA, 0.0f, dir);
                actionAOnlyBoxes[dir].insert(actionAOnlyBoxes[dir].end(), boxes.begin(), boxes.end());
            }

            printf("ABActions: Direction %d - A only: %zu\n",
                   static_cast<int>(dir),
                   actionAOnlyBoxes[dir].size());
        }
        return;
    }

    // If only action B exists
    if (!actionA && actionB)
    {
        HitBox *hitboxB = actionB->getHitBox();
        if (!hitboxB)
        {
            printf("ABActions: Action B has no hitbox\n");
            return;
        }

        const std::vector<HitboxTile> &tilesB = hitboxB->getTiles();

        for (Direction dir : {Direction::UP, Direction::DOWN, Direction::LEFT, Direction::RIGHT})
        {
            std::vector<CF_Aabb> tempBoxesB = hitboxB->getBoxes(dir, cf_v2(0, 0));
            float hitboxSizeB = tempBoxesB.empty() ? 32.0f : (tempBoxesB[0].max.x - tempBoxesB[0].min.x);

            for (const auto &tile : tilesB)
            {
                std::vector<HitboxTile> singleTile = {tile};
                std::vector<CF_Aabb> boxes = HitBox::buildFromTiles(singleTile, hitboxSizeB, 0.0f, dir);
                actionBOnlyBoxes[dir].insert(actionBOnlyBoxes[dir].end(), boxes.begin(), boxes.end());
            }

            printf("ABActions: Direction %d - B only: %zu\n",
                   static_cast<int>(dir),
                   actionBOnlyBoxes[dir].size());
        }
        return;
    }

    // Both actions exist - calculate overlap
    HitBox *hitboxA = actionA->getHitBox();
    HitBox *hitboxB = actionB->getHitBox();

    if (!hitboxA || !hitboxB)
    {
        printf("ABActions: Cannot calculate - one or both actions have no hitbox\n");
        return;
    }

    // Calculate for all 4 directions
    for (Direction dir : {Direction::UP, Direction::DOWN, Direction::LEFT, Direction::RIGHT})
    {
        // Get the tiles from each action's hitbox
        const std::vector<HitboxTile> &tilesA = hitboxA->getTiles();
        const std::vector<HitboxTile> &tilesB = hitboxB->getTiles();

        // Get hitbox size from the first box (all boxes should be same size)
        std::vector<CF_Aabb> tempBoxesA = hitboxA->getBoxes(dir, cf_v2(0, 0));
        float hitboxSizeA = tempBoxesA.empty() ? 32.0f : (tempBoxesA[0].max.x - tempBoxesA[0].min.x);

        std::vector<CF_Aabb> tempBoxesB = hitboxB->getBoxes(dir, cf_v2(0, 0));
        float hitboxSizeB = tempBoxesB.empty() ? 32.0f : (tempBoxesB[0].max.x - tempBoxesB[0].min.x);

        // Track which tiles have been matched as overlapping
        std::vector<bool> tileAMatched(tilesA.size(), false);
        std::vector<bool> tileBMatched(tilesB.size(), false);

        // Find overlapping tiles based on x,y coordinates
        for (size_t i = 0; i < tilesA.size(); ++i)
        {
            for (size_t j = 0; j < tilesB.size(); ++j)
            {
                if (tileBMatched[j])
                    continue;

                // Tiles overlap if they have the same x,y coordinates
                if (tilesA[i].x == tilesB[j].x && tilesA[i].y == tilesB[j].y)
                {
                    // Create a box for this overlapping tile
                    std::vector<HitboxTile> overlapTile = {tilesA[i]};
                    std::vector<CF_Aabb> overlapBox = HitBox::buildFromTiles(overlapTile, hitboxSizeA, 0.0f, dir);
                    bothActionBoxes[dir].insert(bothActionBoxes[dir].end(), overlapBox.begin(), overlapBox.end());

                    tileAMatched[i] = true;
                    tileBMatched[j] = true;
                    break;
                }
            }
        }

        // Add non-overlapping tiles from A
        for (size_t i = 0; i < tilesA.size(); ++i)
        {
            if (!tileAMatched[i])
            {
                std::vector<HitboxTile> singleTile = {tilesA[i]};
                std::vector<CF_Aabb> boxes = HitBox::buildFromTiles(singleTile, hitboxSizeA, 0.0f, dir);
                actionAOnlyBoxes[dir].insert(actionAOnlyBoxes[dir].end(), boxes.begin(), boxes.end());
            }
        }

        // Add non-overlapping tiles from B
        for (size_t i = 0; i < tilesB.size(); ++i)
        {
            if (!tileBMatched[i])
            {
                std::vector<HitboxTile> singleTile = {tilesB[i]};
                std::vector<CF_Aabb> boxes = HitBox::buildFromTiles(singleTile, hitboxSizeB, 0.0f, dir);
                actionBOnlyBoxes[dir].insert(actionBOnlyBoxes[dir].end(), boxes.begin(), boxes.end());
            }
        }

        printf("ABActions: Direction %d - A only: %zu, B only: %zu, Both: %zu\n",
               static_cast<int>(dir),
               actionAOnlyBoxes[dir].size(),
               actionBOnlyBoxes[dir].size(),
               bothActionBoxes[dir].size());
    }
}

void ABActions::render(v2 position, Direction direction, CF_Color colorA, CF_Color colorB,
                       float border_opacity, float fill_opacity)
{
    // Render action A only boxes
    if (actionAOnlyBoxes.count(direction))
    {
        for (const auto &box : actionAOnlyBoxes[direction])
        {
            CF_Aabb translated = box;
            translated.min += position;
            translated.max += position;
            highlightArea(translated, colorA, border_opacity, fill_opacity);
        }
    }

    // Render action B only boxes
    if (actionBOnlyBoxes.count(direction))
    {
        for (const auto &box : actionBOnlyBoxes[direction])
        {
            CF_Aabb translated = box;
            translated.min += position;
            translated.max += position;
            highlightArea(translated, colorB, border_opacity, fill_opacity);
        }
    }

    // Render both actions boxes with split colors
    if (bothActionBoxes.count(direction))
    {
        for (const auto &box : bothActionBoxes[direction])
        {
            CF_Aabb translated = box;
            translated.min += position;
            translated.max += position;
            highlightAreaHalves(translated, colorA, colorB, border_opacity, fill_opacity);
        }
    }
}
