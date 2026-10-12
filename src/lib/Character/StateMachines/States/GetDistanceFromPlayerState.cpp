#include "GetDistanceFromPlayerState.h"
#include <cstdlib>
#include <ctime>
#include <cstdio>

GetDistanceFromPlayerState::GetDistanceFromPlayerState()
    : State()
{
}

GetDistanceFromPlayerState::~GetDistanceFromPlayerState()
{
}

void GetDistanceFromPlayerState::initFromJson()
{
    // Call parent implementation first
    State::initFromJson();
}

void GetDistanceFromPlayerState::update(float dt)
{
    // TODO get distance from player, store it in registerB
    // todo then say is finished
}
