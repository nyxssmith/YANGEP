#pragma once

#include <cstddef>

// Forward declarations
class CFNativeCamera;
class DataFile;
class AnimatedDataCharacter;
class AnimatedDataCharacterNavMeshAgent;
struct StructureLayer;

/**
 * ObjectRenderedByWorldPosition - Represents an object that can be rendered based on world Y position
 *
 * This class can hold one of three types:
 * - StructureLayer (type 0)
 * - NavMeshAgent (type 1)
 * - PlayerCharacter (type 2)
 */
class ObjectRenderedByWorldPosition
{
private:
    int type; // 0 = StructureLayer, 1 = NavMeshAgent, 2 = PlayerCharacter

    // Storage for the different types (only one will be valid based on type)
    StructureLayer *structureLayer;
    AnimatedDataCharacterNavMeshAgent *navMeshAgent;
    AnimatedDataCharacter *playerCharacter;

public:
    /**
     * Constructor for StructureLayer
     */
    explicit ObjectRenderedByWorldPosition(StructureLayer *layer);

    /**
     * Constructor for NavMeshAgent
     */
    explicit ObjectRenderedByWorldPosition(AnimatedDataCharacterNavMeshAgent *agent);

    /**
     * Constructor for PlayerCharacter
     */
    explicit ObjectRenderedByWorldPosition(AnimatedDataCharacter *player);

    /**
     * Get the type of object
     * @return 0 for StructureLayer, 1 for NavMeshAgent, 2 for PlayerCharacter
     */
    int getType() const { return type; }

    /**
     * Get as StructureLayer (only valid if type == 0)
     */
    StructureLayer *asStructureLayer() const { return structureLayer; }

    /**
     * Get as NavMeshAgent (only valid if type == 1)
     */
    AnimatedDataCharacterNavMeshAgent *asNavMeshAgent() const { return navMeshAgent; }

    /**
     * Get as PlayerCharacter (only valid if type == 2)
     */
    AnimatedDataCharacter *asPlayerCharacter() const { return playerCharacter; }

    /**
     * Render the object using the appropriate render method based on type
     * @param camera Camera to use for rendering
     * @param config Configuration data file
     * @param player Player character (for context, may not be used by all types)
     * @param worldX World X offset
     * @param worldY World Y offset
     */
    void render(const CFNativeCamera &camera, const DataFile &config, AnimatedDataCharacter *player = nullptr, float worldX = 0.0f, float worldY = 0.0f);
};

/**
 * WorldPositionRenderedObjectsList - Linked list for objects rendered by world Y position
 *
 * This class maintains a linked list of ObjectRenderedByWorldPosition nodes.
 * Objects can be added, removed, and sorted by their world Y position for proper depth ordering.
 */
class WorldPositionRenderedObjectsList
{
private:
    struct Node
    {
        ObjectRenderedByWorldPosition object;
        Node *next;
        Node *prev;

        Node(const ObjectRenderedByWorldPosition &obj)
            : object(obj), next(nullptr), prev(nullptr) {}
    };

    Node *head;
    Node *tail;
    size_t count;

public:
    /**
     * Constructor
     */
    WorldPositionRenderedObjectsList();

    /**
     * Destructor - cleans up all nodes
     */
    ~WorldPositionRenderedObjectsList();

    /**
     * Add an object to the list
     * @param object The object to add
     */
    void add(const ObjectRenderedByWorldPosition &object);

    /**
     * Remove an object from the list
     * @param object The object to remove (compares pointers)
     * @return true if object was found and removed, false otherwise
     */
    bool remove(const ObjectRenderedByWorldPosition &object);

    /**
     * Sort the list by world Y position
     * For now, this is a placeholder and does nothing
     */
    void sort();

    /**
     * Get the number of objects in the list
     * @return Number of objects
     */
    size_t getCount() const { return count; }

    /**
     * Clear all objects from the list
     */
    void clear();

    /**
     * Iterate through all objects in the list and call a function on each
     * @param func Function to call for each ObjectRenderedByWorldPosition
     */
    template <typename Func>
    void forEach(Func func)
    {
        Node *current = head;
        while (current)
        {
            func(current->object);
            current = current->next;
        }
    }
};
