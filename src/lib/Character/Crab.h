#ifndef CRAB_H
#define CRAB_H

#include "AnimatedDataCharacter.h"

// Crab character class extending AnimatedDataCharacter with claw items
class Crab : public AnimatedDataCharacter
{
public:
    Crab();
    virtual ~Crab();

    // Initialize the crab character
    bool init(const std::string &folderPath);

    // Get left claw item (nullptr if empty)
    Item* getLeftClaw();

    // Set left claw item
    void setLeftClaw(Item* item);

    // Get right claw item (nullptr if empty)
    Item* getRightClaw();

    // Set right claw item
    void setRightClaw(Item* item);

    // Get the left claw slot (for internal use)
    std::optional<Item>& getLeftClawSlot();

    // Get the right claw slot (for internal use)
    std::optional<Item>& getRightClawSlot();

    // Remove left claw item
    bool removeLeftClaw();

    // Remove right claw item
    bool removeRightClaw();

private:
    // Claw slots for inventory (2 slots)
    std::vector<std::optional<Item>> clawSlots;

    // Internal method to add item to a specific slot
    void addItem(std::vector<std::optional<Item>> &slots, size_t index, const Item &itemToAdd);

    // Get available slot count for claw management
    size_t getAvailableSlotCount() const;

    // Internal method to add item to a claw slot
    void addClawItem(size_t index, const Item &itemToAdd);
};

#endif // CRAB_H
