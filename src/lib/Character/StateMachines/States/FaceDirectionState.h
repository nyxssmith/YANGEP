#ifndef FACE_DIRECTION_STATE_H
#define FACE_DIRECTION_STATE_H

#include "State.h"
#include <string>
#include "../../FileHandling/SpriteAnimationLoader.h"

class FaceDirectionState : public State
{
public:
    FaceDirectionState();
    ~FaceDirectionState();

    // Override update to print and complete
    void update(float dt) override;

protected:
    // Override initFromJson to load direction from inputs
    void initFromJson() override;

private:
    Direction direction; // Direction to face
};

#endif // FACE_DIRECTION_STATE_H
