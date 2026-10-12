#ifndef SET_REGISTER_STATE_H
#define SET_REGISTER_STATE_H

#include "State.h"
#include <string>

enum RegisterToSet
{
    REGISTER_A,
    REGISTER_B,
    REGISTER_C
};

class SetRegisterState : public State
{
public:
    SetRegisterState();
    ~SetRegisterState();

    // Override update to print and complete
    void update(float dt) override;

    // Get the register value to set
    const std::string &getToSet() const;

protected:
    // Override initFromJson to load to_set from inputs
    void initFromJson() override;

private:
    RegisterToSet to_set; // Register to set
    bool A_value; // Value for register A
    float B_value; // Value for register B
    std::string C_value; // Value for register C
};

#endif // SET_REGISTER_STATE_H
