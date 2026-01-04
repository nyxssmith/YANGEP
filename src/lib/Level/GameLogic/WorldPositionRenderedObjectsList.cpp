#include "WorldPositionRenderedObjectsList.h"
#include "LevelMap.h"
#include "AnimatedDataCharacter.h"
#include "AnimatedDataCharacterNavMeshAgent.h"
#include "CFNativeCamera.h"
#include "DataFile.h"
#include <cstdio>

// ObjectRenderedByWorldPosition implementation
ObjectRenderedByWorldPosition::ObjectRenderedByWorldPosition(StructureLayer *layer)
    : type(0), worldY(0.0f), structureLayer(layer), navMeshAgent(nullptr), playerCharacter(nullptr)
{
}

ObjectRenderedByWorldPosition::ObjectRenderedByWorldPosition(AnimatedDataCharacterNavMeshAgent *agent)
    : type(1), worldY(0.0f), structureLayer(nullptr), navMeshAgent(agent), playerCharacter(nullptr)
{
}

ObjectRenderedByWorldPosition::ObjectRenderedByWorldPosition(AnimatedDataCharacter *player)
    : type(2), worldY(0.0f), structureLayer(nullptr), navMeshAgent(nullptr), playerCharacter(player)
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
    // Calculate worldY for dynamic objects only (structures are calculated once when added)
    Node *current = head;
    int tileWidth = 32;  // TODO: Get actual tile width from somewhere
    int tileHeight = 32; // TODO: Get actual tile height from somewhere

    while (current)
    {
        ObjectRenderedByWorldPosition &obj = current->object;

        switch (obj.getType())
        {
        case 0: // StructureLayer
        {
            // Skip - structure positions are calculated once when added since they never move
            break;
        }

        case 1: // NavMeshAgent
        {
            AnimatedDataCharacterNavMeshAgent *agent = obj.asNavMeshAgent();
            if (agent)
            {
                v2 pos = agent->getPosition();
                // WorldY = agent's worldY - (tile_height/2)
                obj.setWorldY(pos.y - (tileHeight / 2.0f));
            }
            break;
        }

        case 2: // PlayerCharacter
        {
            AnimatedDataCharacter *player = obj.asPlayerCharacter();
            if (player)
            {
                v2 pos = player->getPosition();
                // WorldY = player's worldY - (tile_height/2)
                obj.setWorldY(pos.y - (tileHeight / 2.0f));
            }
            break;
        }
        }

        current = current->next;
    }

    // Now perform actual sorting using insertion sort
    // (works well for linked lists and nearly-sorted data)
    if (!head || !head->next)
    {
        return; // Already sorted (0 or 1 element)
    }

    Node *sorted = nullptr;
    current = head;

    while (current)
    {
        Node *next = current->next;

        // Insert current node into sorted list
        if (!sorted || sorted->object.getWorldY() <= current->object.getWorldY())
        {
            // Insert at beginning
            current->next = sorted;
            current->prev = nullptr;
            if (sorted)
            {
                sorted->prev = current;
            }
            sorted = current;
        }
        else
        {
            // Find insertion point
            Node *search = sorted;
            while (search->next && search->next->object.getWorldY() > current->object.getWorldY())
            {
                search = search->next;
            }

            // Insert after search
            current->next = search->next;
            current->prev = search;
            if (search->next)
            {
                search->next->prev = current;
            }
            search->next = current;
        }

        current = next;
    }

    // Update head and tail
    head = sorted;
    tail = head;
    while (tail && tail->next)
    {
        tail = tail->next;
    }
}

void WorldPositionRenderedObjectsList::debugPrint() const
{
    printf("\n╔══════════════════════════════════════════════════════════════════════╗\n");
    printf("║        World Position Rendered Objects List - Debug Output          ║\n");
    printf("╠══════════════════════════════════════════════════════════════════════╣\n");
    printf("║ Total Objects: %-53zu ║\n", count);
    printf("╚══════════════════════════════════════════════════════════════════════╝\n\n");

    int objectNum = 1;
    Node *current = head;

    while (current)
    {
        const ObjectRenderedByWorldPosition &obj = current->object;

        printf("┌─ Object #%d ", objectNum++);
        for (int i = 0; i < 60; i++)
            printf("─");
        printf("\n");

        switch (obj.getType())
        {
        case 0: // StructureLayer
        {
            StructureLayer *structure = obj.asStructureLayer();
            if (structure)
            {
                printf("│ Type: StructureLayer\n");
                printf("│ Name: %s\n", structure->name.c_str());
                printf("│ Dimensions: %d x %d tiles\n", structure->width, structure->height);
                printf("│ Lowest World Y: %d\n", structure->lowestWorldYCoordinate);
                printf("│\n");
                printf("│ Tile Coordinates (World Space):\n");

                int tileCount = 0;
                int tileWidth = 32;  // TODO: Get actual tile width from somewhere
                int tileHeight = 32; // TODO: Get actual tile height from somewhere

                for (int y = 0; y < structure->height; y++)
                {
                    for (int x = 0; x < structure->width; x++)
                    {
                        int gid = structure->getTileGID(x, y);
                        if (gid != 0) // Non-empty tile
                        {
                            // Convert tile coordinates to world coordinates
                            float worldX = x * tileWidth;
                            float worldY = ((structure->height - 1 - y) * tileHeight);

                            if (tileCount > 0 && tileCount % 3 == 0)
                                printf("\n");

                            printf("│   [%3d,%3d] → (%.1f, %.1f)  ", x, y, worldX, worldY);
                            tileCount++;
                        }
                    }
                }

                if (tileCount > 0)
                    printf("\n");

                printf("│ Total Tiles: %d\n", tileCount);
            }
            break;
        }

        case 1: // NavMeshAgent
        {
            AnimatedDataCharacterNavMeshAgent *agent = obj.asNavMeshAgent();
            if (agent)
            {
                v2 pos = agent->getPosition();
                printf("│ Type: NavMeshAgent\n");
                printf("│ Position: (%.2f, %.2f)\n", pos.x, pos.y);
                printf("│ On Screen: %s\n", agent->getIsOnScreen() ? "Yes" : "No");
                printf("│ Current Polygon: %d\n", agent->getCurrentPolygon());
            }
            break;
        }

        case 2: // PlayerCharacter
        {
            AnimatedDataCharacter *player = obj.asPlayerCharacter();
            if (player)
            {
                v2 pos = player->getPosition();
                printf("│ Type: PlayerCharacter\n");
                printf("│ Position: (%.2f, %.2f)\n", pos.x, pos.y);
                printf("│ Current Direction: %d\n", player->getCurrentDirection());
            }
            break;
        }

        default:
            printf("│ Type: Unknown (%d)\n", obj.getType());
            break;
        }

        printf("└");
        for (int i = 0; i < 70; i++)
            printf("─");
        printf("\n\n");

        current = current->next;
    }

    printf("═════════════════════════════════════════════════════════════════════════\n");
    printf("  End of World Position Rendered Objects List\n");
    printf("═════════════════════════════════════════════════════════════════════════\n\n");
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
