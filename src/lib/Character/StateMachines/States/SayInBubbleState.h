#ifndef SAY_IN_BUBBLE_STATE_H
#define SAY_IN_BUBBLE_STATE_H

#include "State.h"
#include <string>

class SayInBubbleState : public State
{
public:
    SayInBubbleState();
    ~SayInBubbleState();

    // Override update to print and complete
    void update(float dt) override;

    // Get the text to print
    const std::string &getToSay() const;

protected:
    // Override initFromJson to load to_print from inputs
    void initFromJson() override;

private:
    std::string to_say; // Text to display in the speech bubble
};

#endif // SAY_IN_BUBBLE_STATE_H
