#include "GetDistanceFromHomeState.h"
#include <cstdlib>
#include <ctime>
#include <cstdio>

GetDistanceFromHomeState::GetDistanceFromHomeState()
    : State()
{
}

GetDistanceFromHomeState::~GetDistanceFromHomeState()
{
}

void GetDistanceFromHomeState::initFromJson()
{
    // Call parent implementation first
    State::initFromJson();
}

void GetDistanceFromHomeState::update(float dt)
{
    // TODO get distance from home, store it in registerB
    // todo then say is finished
}
