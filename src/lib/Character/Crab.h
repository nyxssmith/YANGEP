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
    Item *getLeftClaw();

    // Set left claw item
    void setLeftClaw(Item *item);

    // Get right claw item (nullptr if empty)
    Item *getRightClaw();

    // Set right claw item
    void setRightClaw(Item *item);

    // Get the left claw slot (for internal use)
    Item *&getLeftClawSlot();

    // Get the right claw slot (for internal use)
    Item *&getRightClawSlot();

    // Remove left claw item
    bool removeLeftClaw();

    // Remove right claw item
    bool removeRightClaw();

    void doClawAction(int clawIndex);

    // Tick the provided actions of equipped claws (they live in the items)
    void updateItemActions(float dt) override;

    void applyGlobalCooldown(float globalCooldownSeconds) override;
    void doRightClawAction();
    void doLeftClawAction();

    // Add an item to the inventory (currently defers to the parent)
    bool addItemToInventory(std::string fullPath) override;

private:
    // Non-owning pointers to inventory items (2 slots)
    std::vector<Item *> clawSlots;

    // Get available slot count for claw management
    size_t getAvailableSlotCount() const;

    // Internal method to add item to a claw slot
    void addClawItem(size_t index, Item *itemToAdd);
};

#endif // CRAB_H
