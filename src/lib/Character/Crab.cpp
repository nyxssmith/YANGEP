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
    : AnimatedDataCharacter(), clawSlots(2) // Initialize with 2 claw slots
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
    clawSlots[0] = std::nullopt;
    clawSlots[1] = std::nullopt;
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

    // Clean up claw item pointers if they exist
    if (getLeftClaw())
    {
        delete getLeftClaw();
    }
    if (getRightClaw())
    {
        delete getRightClaw();
    }
}

// Initialize the crab character with a folder path containing character.json
bool Crab::init(const std::string &folderPath)
{
    // Call parent init to load animations and set up basic character state
    if (!AnimatedDataCharacter::init(folderPath))
    {
        return false;
    }

    // Initialize claw slots as empty (already done in constructor, but explicit here for clarity)
    clawSlots[0] = std::nullopt;
    clawSlots[1] = std::nullopt;

    setInitialized(true);  // Use setter to set initialized state
    return true;
}

// Get left claw item (nullptr if empty)
Item* Crab::getLeftClaw()
{
    if (clawSlots.empty())
    {
        return nullptr;
    }

    // Check if left claw slot is occupied
    if (!clawSlots[0].has_value())
    {
        return nullptr;
    }

    return &(*clawSlots[0]);
}

// Set left claw item
void Crab::setLeftClaw(Item* item)
{
    if (item)
    {
        // Add new item to slot 0
        addClawItem(0, *item);
    }
}

// Get right claw item (nullptr if empty)
Item* Crab::getRightClaw()
{
    if (clawSlots.empty())
    {
        return nullptr;
    }

    // Check if right claw slot is occupied
    if (!clawSlots[1].has_value())
    {
        return nullptr;
    }

    return &(*clawSlots[1]);
}

// Set right claw item
void Crab::setRightClaw(Item* item)
{
    if (item)
    {
        // Add new item to slot 1
        addClawItem(1, *item);
    }
}

// Remove left claw item
bool Crab::removeLeftClaw()
{
    if (clawSlots.empty())
    {
        return false;
    }

    if (!clawSlots[0].has_value())
    {
        return false;
    }

    Item* existingItem = &(*clawSlots[0]);
    clawSlots[0] = std::nullopt;
    delete existingItem;

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

    if (!clawSlots[1].has_value())
    {
        return false;
    }

    Item* existingItem = &(*clawSlots[1]);
    clawSlots[1] = std::nullopt;
    delete existingItem;

    printf("Crab: Removed right claw item\n");
    return true;
}

// Helper method to add an item to a specific slot in the claw slots vector
void Crab::addItem(std::vector<std::optional<Item>> &slots, size_t index, const Item &itemToAdd)
{
    if (index >= slots.size())
    {
        printf("Crab: WARNING - Cannot add item to slot %zu, max slots: %zu\n", index, slots.size());
        return;
    }

    // If slot is occupied, remove the existing item first
    if (slots[index].has_value())
    {
        Item* existingItem = &(*slots[index]);
        printf("Crab: Slot %zu was occupied, removing existing item\n", index);
        delete existingItem;
        slots[index] = std::nullopt;
    }

    // Add new item to the slot
    slots[index] = Item(itemToAdd);
}

// Internal helper - add item to claw slot
void Crab::addClawItem(size_t index, const Item &itemToAdd)
{
    if (index >= clawSlots.size())
    {
        printf("Crab: WARNING - Cannot add item to slot %zu, max slots: %zu\n", index, clawSlots.size());
        return;
    }

    // If slot is occupied, remove the existing item first
    if (clawSlots[index].has_value())
    {
        Item* existingItem = &(*clawSlots[index]);
        printf("Crab: Slot %zu was occupied, removing existing item\n", index);
        delete existingItem;
        clawSlots[index] = std::nullopt;
    }

    // Add new item to the slot
    clawSlots[index] = Item(itemToAdd);
}

// Get available slot count for claw management
size_t Crab::getAvailableSlotCount() const
{
    return clawSlots.size();
}

std::optional<Item>& Crab::getLeftClawSlot() {
    return clawSlots[0];
}

std::optional<Item>& Crab::getRightClawSlot() {
    return clawSlots[1];
}
