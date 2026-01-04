#include "WorldPositionRenderedObjectsList.h"
#include "LevelMap.h"
#include "AnimatedDataCharacter.h"
#include "AnimatedDataCharacterNavMeshAgent.h"
#include "CFNativeCamera.h"
#include "DataFile.h"
#include <cstdio>

// ObjectRenderedByWorldPosition implementation
ObjectRenderedByWorldPosition::ObjectRenderedByWorldPosition(StructureLayer *layer)
    : type(0), structureLayer(layer), navMeshAgent(nullptr), playerCharacter(nullptr)
{
}

ObjectRenderedByWorldPosition::ObjectRenderedByWorldPosition(AnimatedDataCharacterNavMeshAgent *agent)
    : type(1), structureLayer(nullptr), navMeshAgent(agent), playerCharacter(nullptr)
{
}

ObjectRenderedByWorldPosition::ObjectRenderedByWorldPosition(AnimatedDataCharacter *player)
    : type(2), structureLayer(nullptr), navMeshAgent(nullptr), playerCharacter(player)
{
}

void ObjectRenderedByWorldPosition::render(const CFNativeCamera &camera, const DataFile &config, AnimatedDataCharacter *player, float worldX, float worldY)
{
    switch (type)
    {
    case 0: // StructureLayer
        if (structureLayer && structureLayer->getTMXLayer())
        {
            // Render the structure layer using the TMX layer
            // Note: renderLayer is a method on LevelMap/tmx that renders a single layer
            // For now, we'll need to access this through the level map
            // This is a placeholder - we may need to refactor this
            printf("TODO: Render StructureLayer '%s' (need access to renderLayer)\n", structureLayer->name.c_str());
        }
        break;

    case 1: // NavMeshAgent
        if (navMeshAgent)
        {
            v2 agentPos = navMeshAgent->getPosition();
            navMeshAgent->render(agentPos);
        }
        break;

    case 2: // PlayerCharacter
        if (playerCharacter)
        {
            v2 playerPos = playerCharacter->getPosition();
            playerCharacter->render(playerPos);
        }
        break;

    default:
        printf("ObjectRenderedByWorldPosition: Unknown type %d\n", type);
        break;
    }
}

// WorldPositionRenderedObjectsList implementation
WorldPositionRenderedObjectsList::WorldPositionRenderedObjectsList()
    : head(nullptr), tail(nullptr), count(0)
{
}

WorldPositionRenderedObjectsList::~WorldPositionRenderedObjectsList()
{
    clear();
}

void WorldPositionRenderedObjectsList::add(const ObjectRenderedByWorldPosition &object)
{
    Node *newNode = new Node(object);

    if (!head)
    {
        // Empty list
        head = tail = newNode;
    }
    else
    {
        // Add to end of list
        tail->next = newNode;
        newNode->prev = tail;
        tail = newNode;
    }

    count++;
}

bool WorldPositionRenderedObjectsList::remove(const ObjectRenderedByWorldPosition &object)
{
    Node *current = head;

    while (current)
    {
        // Compare based on type and pointer
        bool match = false;
        if (current->object.getType() == object.getType())
        {
            switch (object.getType())
            {
            case 0: // StructureLayer
                match = (current->object.asStructureLayer() == object.asStructureLayer());
                break;
            case 1: // NavMeshAgent
                match = (current->object.asNavMeshAgent() == object.asNavMeshAgent());
                break;
            case 2: // PlayerCharacter
                match = (current->object.asPlayerCharacter() == object.asPlayerCharacter());
                break;
            }
        }

        if (match)
        {
            // Remove this node
            if (current->prev)
            {
                current->prev->next = current->next;
            }
            else
            {
                // Removing head
                head = current->next;
            }

            if (current->next)
            {
                current->next->prev = current->prev;
            }
            else
            {
                // Removing tail
                tail = current->prev;
            }

            delete current;
            count--;
            return true;
        }

        current = current->next;
    }

    return false;
}

void WorldPositionRenderedObjectsList::sort()
{
    // TODO: Implement sorting by world Y position
    // For now, this is a placeholder
}

void WorldPositionRenderedObjectsList::clear()
{
    Node *current = head;
    while (current)
    {
        Node *next = current->next;
        delete current;
        current = next;
    }

    head = tail = nullptr;
    count = 0;
}
