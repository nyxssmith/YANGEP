#ifndef IF_STATE_H
#define IF_STATE_H

#include "State.h"
#include <string>
enum RegisterToUse
{
    USE_REGISTER_A,
    USE_REGISTER_B,
    USE_REGISTER_C
};
class IfState : public State
{
public:
    IfState();
    ~IfState();

    // Override update to print and complete
    void update(float dt) override;

protected:
    // Override initFromJson to load to_print from inputs
    void initFromJson() override;

private:
    std::string operation;         // input operation
    std::string register_string;   // string for what register to use
    RegisterToUse register_to_use; // enum for what register to use
    bool operand_a;                // value of the right operand if register to use is REGISTER_A
    float operand_b;               // value of the right operand if register to use is REGISTER_B
    std::string operand_c;         // value of the right operand if register to use is REGISTER_C
    bool result;                   // result of the if condition
};

#endif // IF_STATE_H
