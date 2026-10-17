#include "IfState.h"
#include <cstdio>

IfState::IfState()
    : State(), operation(""), register_string(""), register_to_use(USE_REGISTER_A), operand_a(false), operand_b(0.0f), operand_c(""), result(false)
{
}

IfState::~IfState()
{
}

void IfState::initFromJson()
{
    // Call parent implementation first
    State::initFromJson();

    // Try to read 'to_print' from default values
    const DataFile &defaults = getDefaultValues();
    if (defaults.contains("operation"))
    {
        if (defaults["operation"].is_string())
        {
            operation = defaults["operation"].get<std::string>();
            // printf("IfState: Loaded operation: '%s'\n", operation.c_str());
        }
    }
    if (defaults.contains("register"))
    {
        if (defaults["register"].is_string())
        {
            register_string = defaults["register"].get<std::string>();
            // printf("IfState: Loaded register: '%s'\n", register_string.c_str());
            if (register_string == "A")
            {
                register_to_use = USE_REGISTER_A;
            }
            else if (register_string == "B")
            {
                register_to_use = USE_REGISTER_B;
            }
            else if (register_string == "C")
            {
                register_to_use = USE_REGISTER_C;
            }
        }
    }
    if (defaults.contains("value"))
    {
        if (register_to_use == USE_REGISTER_A && defaults["value"].is_boolean())
        {
            operand_a = defaults["value"].get<bool>();
        }
        else if (register_to_use == USE_REGISTER_B && defaults["value"].is_number_float())
        {
            operand_b = defaults["value"].get<float>();
        }
        else if (register_to_use == USE_REGISTER_C && defaults["value"].is_string())
        {
            operand_c = defaults["value"].get<std::string>();
        }
    }
}

void IfState::update(float dt)
{
    if (!getIsRunning())
    {
        return;
    }
    // TODO eval

    // Mark as complete
    setIsRunning(false);
}
