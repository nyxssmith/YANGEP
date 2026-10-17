#include "GetEntitiesInBoxState.h"
#include <cstdlib>
#include <ctime>
#include <cstdio>

GetEntitiesInBoxState::GetEntitiesInBoxState()
    : State()
{
}

GetEntitiesInBoxState::~GetEntitiesInBoxState()
{
}

void GetEntitiesInBoxState::initFromJson()
{
    // Call parent implementation first
    State::initFromJson();
    // Try to read 'to_print' from default values
    const DataFile &defaults = getDefaultValues();
    if (defaults.contains("top_left_tile_x"))
    {
        if (defaults["top_left_tile_x"].is_number_integer())
        {
            top_left_tile_x = defaults["top_left_tile_x"].get<int>();
        }
    }
    if (defaults.contains("top_left_tile_y"))
    {
        if (defaults["top_left_tile_y"].is_number_integer())
        {
            top_left_tile_y = defaults["top_left_tile_y"].get<int>();
        }
    }
    if (defaults.contains("bottom_right_tile_x"))
    {
        if (defaults["bottom_right_tile_x"].is_number_integer())
        {
            bottom_right_tile_x = defaults["bottom_right_tile_x"].get<int>();
        }
    }
    if (defaults.contains("bottom_right_tile_y"))
    {
        if (defaults["bottom_right_tile_y"].is_number_integer())
        {
            bottom_right_tile_y = defaults["bottom_right_tile_y"].get<int>();
        }
    }
}

void GetEntitiesInBoxState::update(float dt)
{
    // TODO get entities in box, store them in registerB
    // todo then say is finished
}
