#ifndef PRINT_STATE_H
#define PRINT_STATE_H

#include "State.h"
#include <string>

class PrintState : public State
{
public:
    PrintState();
    ~PrintState();

    // Override update to print and complete
    void update(float dt) override;

    // Get the text to print
    const std::string &getToPrint() const;

protected:
    // Override initFromJson to load to_print from inputs
    void initFromJson() override;

private:
    std::string to_print; // Text to print to stdout
};

#endif // PRINT_STATE_H
