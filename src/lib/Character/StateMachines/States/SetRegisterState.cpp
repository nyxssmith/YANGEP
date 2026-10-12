#include "SetRegisterState.h"
#include <cstdio>

SetRegisterState::SetRegisterState()
    : State(), to_set(REGISTER_A), A_value(false), B_value(0.0f), C_value("")
{
}

SetRegisterState::~SetRegisterState()
{
}

void SetRegisterState::initFromJson()
{
    // Call parent implementation first
    State::initFromJson();

    // Try to read 'to_print' from default values
    const DataFile &defaults = getDefaultValues();
    if (defaults.contains("register"))
    {
        if (defaults["register"].is_string())
        {
            std::string reg = defaults["register"].get<std::string>();
            if (reg == "A")
                to_set = REGISTER_A;
            else if (reg == "B")
                to_set = REGISTER_B;
            else if (reg == "C")
                to_set = REGISTER_C;
            // printf("SetRegisterState: Loaded register to set: '%s'\n", reg.c_str());
        }
    }
    if (defaults.contains("value"))
    {
        if (defaults["value"].is_boolean() && to_set == REGISTER_A)
        {
            A_value = defaults["value"].get<bool>();
        }
        else if (defaults["value"].is_number() && to_set == REGISTER_B)
        {
            B_value = defaults["value"].get<float>();
        }
        else if (defaults["value"].is_string() && to_set == REGISTER_C)
        {
            C_value = defaults["value"].get<std::string>();
        }
    }
}

void SetRegisterState::update(float dt)
{
    if (!getIsRunning())
    {
        return;
    }
    // TODO this
    switch (to_set)
    {
    case REGISTER_A:
        stateMachine->setRegisterA(A_value);
        break;
    case REGISTER_B:
        stateMachine->setRegisterB(B_value);
        break;
    case REGISTER_C:
        stateMachine->setRegisterC(C_value);
        break;

    default:
        break;
    }

    // Mark as complete
    setIsRunning(false);
}

const std::string &SetRegisterState::getToSet() const
{
    switch (to_set)
    {
    case REGISTER_A:
        return A_value ? "true" : "false";
    case REGISTER_B:
        return std::to_string(B_value);
    case REGISTER_C:
        return C_value;
    default:
        return C_value; // Default case
    }
}
