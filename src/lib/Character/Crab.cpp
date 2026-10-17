#include <cute.h>
#include <cute_draw.h>
#include <spng.h>
#include "Crab.h"
#include "Item.h"
#include "DataFile.h"

using namespace Cute;

// Helper function to get PNG dimensions (from AnimatedDataCharacter.cpp)
static bool getPNGDimensions(const std::string &path, uint32_t &width, uint32_t &height)
{
    size_t file_size = 0;
    void *file_data = cf_fs_read_entire_file_to_memory(path.c_str(), &file_size);

    if (!file_data)
    {
        return false;
    }

    spng_ctx *ctx = spng_ctx_new(0);
    if (!ctx)
    {
        cf_free(file_data);
        return false;
    }

    spng_set_png_buffer(ctx, file_data, file_size);

    struct spng_ihdr ihdr;
    int ret = spng_get_ihdr(ctx, &ihdr);

    if (ret == 0)
    {
        width = ihdr.width;
        height = ihdr.height;
    }

    spng_ctx_free(ctx);
    cf_free(file_data);

    return ret == 0;
}

// Constructor
Crab::Crab()
    : AnimatedDataCharacter(), clawSlots(2, nullptr) // 2 non-owning claw slots pointing into the inventory
{
    // Initialize input state using accessors
    for (int i = 0; i < 4; i++)
    {
        setKeysPressed(i, false);
    }
    for (int i = 0; i < 2; i++)
    {
        setAnimationKeys(i, false);
    }

    // Set default claw slots as empty
    clawSlots[0] = nullptr;
    clawSlots[1] = nullptr;
}

// Destructor
Crab::~Crab()
{
    // Cleanup character hitbox if it exists
    HitBox *hitbox = getCharacterHitbox();
    if (hitbox)
    {
        delete hitbox;
    }

    // Claw slots are non-owning pointers into the inventory, nothing to free
}

// Initialize the crab character with a folder path containing character.json
bool Crab::init(const std::string &folderPath)
{
    // Call parent init to load animations and set up basic character state
    if (!AnimatedDataCharacter::init(folderPath))
    {
        return false;
    }

    // Claw slots are filled during the parent init (via addItemToInventory), so don't reset them here
    setInitialized(true);
    return true;
}

// Get left claw item (nullptr if empty)
Item *Crab::getLeftClaw()
{
    if (clawSlots.empty())
    {
        return nullptr;
    }

    return clawSlots[0];
}

// Set left claw item
void Crab::setLeftClaw(Item *item)
{
    if (item)
    {
        // Add new item to slot 1
        addClawItem(1, item);
    }
}

// Get right claw item (nullptr if empty)
Item *Crab::getRightClaw()
{
    if (clawSlots.empty())
    {
        return nullptr;
    }

    return clawSlots[1];
}

// Set right claw item
void Crab::setRightClaw(Item *item)
{
    if (item)
    {
        // Add new item to slot 0
        addClawItem(0, item);
    }
}

// Remove left claw item
bool Crab::removeLeftClaw()
{
    if (clawSlots.empty())
    {
        return false;
    }

    if (!clawSlots[0])
    {
        return false;
    }

    clawSlots[0] = nullptr;

    printf("Crab: Removed left claw item\n");
    return true;
}

// Remove right claw item
bool Crab::removeRightClaw()
{
    if (clawSlots.empty())
    {
        return false;
    }

    if (!clawSlots[1])
    {
        return false;
    }

    clawSlots[1] = nullptr;

    printf("Crab: Removed right claw item\n");
    return true;
}

// Internal helper - add item to claw slot
void Crab::addClawItem(size_t index, Item *itemToAdd)
{
    if (index >= clawSlots.size())
    {
        printf("Crab: WARNING - Cannot add item to slot %zu, max slots: %zu\n", index, clawSlots.size());
        return;
    }

    if (clawSlots[index])
    {
        printf("Crab: Slot %zu was occupied, replacing claw item\n", index);
    }

    clawSlots[index] = itemToAdd;

    // The provided action needs to know its owner to drive animations and cooldowns
    if (itemToAdd && itemToAdd->hasProvidedAction())
    {
        itemToAdd->getProvidedAction()->setCharacter(this);
    }
}

void Crab::applyGlobalCooldown(float globalCooldownSeconds)
{
    AnimatedDataCharacter::applyGlobalCooldown(globalCooldownSeconds);
    for (Item *claw : clawSlots)
    {
        if (claw && claw->hasProvidedAction())
        {
            claw->getProvidedAction()->applyCooldown(globalCooldownSeconds);
        }
    }
}

// Get available slot count for claw management
size_t Crab::getAvailableSlotCount() const
{
    return clawSlots.size();
}

Item *&Crab::getLeftClawSlot()
{
    return clawSlots[0];
}

Item *&Crab::getRightClawSlot()
{
    return clawSlots[1];
}

bool Crab::addItemToInventory(std::string fullPath)
{
    // get size of inventory before adding
    size_t inventorySizeBefore = getInventory().getItemCount();
    // try to add the item
    bool added_successfully = AnimatedDataCharacter::addItemToInventory(fullPath);

    // TODO different path for item pickup etc
    //  this will only be called during init, add by index is used to pick up items
    if (added_successfully)
    {
        // get the inventory item in the slot it was added to
        Item *addedItem = getInventory().getItemPtr(inventorySizeBefore);
        if (!addedItem)
        {
            return added_successfully;
        }
        // add to left or right claw (non-owning pointer into the inventory)
        if (inventorySizeBefore == 0)
        {
            setActionPointerA(inventorySizeBefore);
            setRightClaw(addedItem);
        }
        else if (inventorySizeBefore == 1)
        {
            setActionPointerB(inventorySizeBefore);
            setLeftClaw(addedItem);
        }
        else
        {
            printf("Crab: Inventory slot %zu does not correspond to a claw slot and will not be accessible via claw actions\n", inventorySizeBefore);
        }
    }
    return added_successfully;
}
void Crab::doRightClawAction()
{
    doClawAction(1);
}

void Crab::doLeftClawAction()
{
    doClawAction(0);
}

void Crab::updateItemActions(float dt)
{
    for (Item *claw : clawSlots)
    {
        if (claw && claw->hasProvidedAction())
        {
            Action *action = claw->getProvidedAction();
            if (action->getIsActive())
            {
                action->update(dt);
            }
        }
    }
}

void Crab::doClawAction(int clawIndex)
{
    if (clawIndex >= clawSlots.size())
    {
        printf("Crab: Invalid claw index %d\n", clawIndex);
        return;
    }

    if (clawSlots[clawIndex])
    {
        printf("Crab: Performing action with claw %d\n", clawIndex);
        bool success = false;
        switch (clawIndex)
        {
        case 0:
            success = getLeftClaw()->doProvidedAction();
            break;
        case 1:
            success = getRightClaw()->doProvidedAction();
            break;

        default:
            break;
        }

        if (!success)
        {
            printf("Crab: Action with claw %d failed, perhaps that item doesnt provide an action\n", clawIndex);
        }
    }
    else
    {
        printf("Crab: No item in claw %d to perform action\n", clawIndex);
    }
}