#include "tmx.h"
#include <cute.h>
#include <functional>
#include <sstream>
#include <algorithm>

tmx::tmx(const std::string &path) : path(path), map_width(0), map_height(0), tile_width(32), tile_height(32)
{
    parse(path);
}

bool tmx::parse(const std::string &path)
{
    printf("Attempting to read TMX file: %s\n", path.c_str());

    // Read the entire file using Cute Framework's VFS
    size_t file_size = 0;
    void *file_data = cf_fs_read_entire_file_to_memory(path.c_str(), &file_size);

    if (file_data == nullptr)
    {
        printf("Failed to read TMX file: %s (file_data is null)\n", path.c_str());
        printf("Check if:\n");
        printf("  1. The file exists at the specified path\n");
        printf("  2. The file system is properly mounted\n");
        printf("  3. The path format is correct (use forward slashes)\n");
        return false;
    }

    if (file_size == 0)
    {
        printf("Failed to read TMX file: %s (file_size is 0)\n", path.c_str());
        cf_free(file_data);
        return false;
    }

    printf("Successfully read %zu bytes from TMX file: %s\n", file_size, path.c_str());

    // Parse the XML data using pugixml
    pugi::xml_parse_result result = load_buffer(file_data, file_size);

    // Free the memory allocated by cf_fs_read_entire_file_to_memory
    cf_free(file_data);

    if (!result)
    {
        printf("XML parsing failed for TMX file '%s': %s at offset %td\n",
               path.c_str(), result.description(), result.offset);
        return false;
    }

    // Store the path after successful parsing
    this->path = path;

    // Parse map properties
    pugi::xml_node map_node = document_element();
    if (strcmp(map_node.name(), "map") != 0)
    {
        printf("Invalid TMX file: root element is not 'map'\n");
        return false;
    }

    // Read map attributes
    map_width = map_node.attribute("width").as_int(0);
    map_height = map_node.attribute("height").as_int(0);
    tile_width = map_node.attribute("tilewidth").as_int(32);
    tile_height = map_node.attribute("tileheight").as_int(32);

    printf("TMX Map properties: %dx%d tiles, %dx%d pixels per tile\n",
           map_width, map_height, tile_width, tile_height);

    // Load tilesets and layers
    if (!loadTilesets())
    {
        printf("Failed to load tilesets from TMX file\n");
        return false;
    }

    if (!loadLayers())
    {
        printf("Failed to load layers from TMX file\n");
        return false;
    }

    printf("Successfully parsed TMX file: %s (%d tilesets, %d layers)\n",
           path.c_str(), static_cast<int>(tilesets.size()), static_cast<int>(layers.size()));

    return true;
}

bool tmx::loadTilesets()
{
    pugi::xml_node map_node = document_element();

    for (pugi::xml_node tileset_node : map_node.children("tileset"))
    {
        auto tileset = std::make_shared<TMXTileset>();

        tileset->first_gid = tileset_node.attribute("firstgid").as_int(1);
        tileset->source = tileset_node.attribute("source").value();
        tileset->name = tileset_node.attribute("name").value();

        printf("Loading tileset: firstgid=%d, source=%s, name=%s\n",
               tileset->first_gid, tileset->source.c_str(), tileset->name.c_str());

        // Load external TSX file if source is specified
        if (!tileset->source.empty())
        {
            // Construct full path to TSX file (relative to TMX file location)
            std::string tsx_path;
            size_t last_slash = path.find_last_of("/\\");
            if (last_slash != std::string::npos)
            {
                tsx_path = path.substr(0, last_slash + 1) + tileset->source;
            }
            else
            {
                tsx_path = tileset->source;
            }

            // Create and load the TSX file
            tileset->tsx_data = std::make_shared<tsx>(tsx_path);

            if (tileset->tsx_data->empty())
            {
                printf("Failed to load TSX file: %s\n", tsx_path.c_str());
                continue;
            }

            printf("Successfully loaded TSX file: %s\n", tsx_path.c_str());
        }
        else
        {
            // Inline tileset data (not implemented for now)
            printf("Warning: Inline tileset data not supported yet\n");
            continue;
        }

        tilesets.push_back(tileset);
    }

    printf("Loaded %d tilesets\n", static_cast<int>(tilesets.size()));
    return !tilesets.empty();
}

bool tmx::loadLayers()
{
    pugi::xml_node map_node = document_element();

    for (pugi::xml_node layer_node : map_node.children("layer"))
    {
        auto layer = std::make_shared<TMXLayer>();

        layer->id = layer_node.attribute("id").as_int(0);
        layer->name = layer_node.attribute("name").value();
        layer->width = layer_node.attribute("width").as_int(map_width);
        layer->height = layer_node.attribute("height").as_int(map_height);
        layer->visible = layer_node.attribute("visible").as_bool(true);
        layer->opacity = layer_node.attribute("opacity").as_float(1.0f);

        printf("Loading layer: id=%d, name=%s, size=%dx%d, visible=%s, opacity=%.2f\n",
               layer->id, layer->name.c_str(), layer->width, layer->height,
               layer->visible ? "true" : "false", layer->opacity);

        // Load layer data
        pugi::xml_node data_node = layer_node.child("data");
        if (!data_node)
        {
            printf("Warning: Layer '%s' has no data node\n", layer->name.c_str());
            continue;
        }

        std::string encoding = data_node.attribute("encoding").value();

        if (encoding == "csv" || encoding.empty())
        {
            // Parse CSV data
            std::string csv_data = data_node.text().get();
            parseCSVData(csv_data, layer->data);
        }
        else
        {
            printf("Warning: Unsupported encoding '%s' for layer '%s'\n",
                   encoding.c_str(), layer->name.c_str());
            continue;
        }

        printf("Layer '%s' loaded with %d tiles\n",
               layer->name.c_str(), static_cast<int>(layer->data.size()));

        layers.push_back(layer);
    }

    printf("Loaded %d layers\n", static_cast<int>(layers.size()));
    return !layers.empty();
}

void tmx::parseCSVData(const std::string &csv_data, std::vector<int> &tile_data) const
{
    tile_data.clear();

    std::istringstream stream(csv_data);
    std::string line;

    while (std::getline(stream, line))
    {
        std::istringstream line_stream(line);
        std::string cell;

        while (std::getline(line_stream, cell, ','))
        {
            // Remove whitespace
            cell.erase(std::remove_if(cell.begin(), cell.end(), ::isspace), cell.end());

            if (!cell.empty())
            {
                int tile_id = std::stoi(cell);
                tile_data.push_back(tile_id);
            }
        }
    }
}

std::shared_ptr<TMXTileset> tmx::findTilesetForGID(int gid) const
{
    if (gid == 0)
        return nullptr; // 0 means no tile

    // Find the tileset that contains this GID
    // Tilesets should be sorted by first_gid, so find the last one where first_gid <= gid
    std::shared_ptr<TMXTileset> best_tileset = nullptr;

    for (const auto &tileset : tilesets)
    {
        if (tileset->first_gid <= gid)
        {
            if (!best_tileset || tileset->first_gid > best_tileset->first_gid)
            {
                best_tileset = tileset;
            }
        }
    }

    return best_tileset;
}

void tmx::debugPrint() const
{
    printf("\n=== TMX Content Analysis ===\n");
    printf("File: %s\n", path.c_str());
    printf("Map size: %dx%d tiles (%dx%d pixels per tile)\n",
           map_width, map_height, tile_width, tile_height);

    printf("\nTilesets (%d):\n", static_cast<int>(tilesets.size()));
    for (size_t i = 0; i < tilesets.size(); i++)
    {
        const auto &tileset = tilesets[i];
        printf("  [%zu] firstgid=%d, source=%s, name=%s\n",
               i, tileset->first_gid, tileset->source.c_str(), tileset->name.c_str());
    }

    printf("\nLayers (%d):\n", static_cast<int>(layers.size()));
    for (size_t i = 0; i < layers.size(); i++)
    {
        const auto &layer = layers[i];
        printf("  [%zu] id=%d, name=%s, size=%dx%d, visible=%s, opacity=%.2f, tiles=%d\n",
               i, layer->id, layer->name.c_str(), layer->width, layer->height,
               layer->visible ? "true" : "false", layer->opacity,
               static_cast<int>(layer->data.size()));
    }

    printf("=== End TMX Content ===\n\n");
}

std::shared_ptr<TMXLayer> tmx::getLayer(int index) const
{
    if (index >= 0 && index < static_cast<int>(layers.size()))
    {
        return layers[index];
    }
    return nullptr;
}

std::shared_ptr<TMXLayer> tmx::getLayer(const std::string &name) const
{
    for (const auto &layer : layers)
    {
        if (layer->name == name)
        {
            return layer;
        }
    }
    return nullptr;
}

std::shared_ptr<TMXTileset> tmx::getTileset(int index) const
{
    if (index >= 0 && index < static_cast<int>(tilesets.size()))
    {
        return tilesets[index];
    }
    return nullptr;
}

CF_Sprite tmx::getTileAt(int layer_index, int map_x, int map_y) const
{
    auto layer = getLayer(layer_index);
    if (!layer)
    {
        printf("Invalid layer index: %d\n", layer_index);
        return cf_sprite_defaults();
    }

    int gid = layer->getTileGID(map_x, map_y);
    if (gid == 0)
    {
        return cf_sprite_defaults(); // Empty tile
    }

    auto tileset = findTilesetForGID(gid);
    if (!tileset)
    {
        printf("No tileset found for GID: %d\n", gid);
        return cf_sprite_defaults();
    }

    return tileset->getSpriteForGID(gid);
}

CF_Sprite tmx::getTileAt(const std::string &layer_name, int map_x, int map_y) const
{
    auto layer = getLayer(layer_name);
    if (!layer)
    {
        printf("Layer not found: %s\n", layer_name.c_str());
        return cf_sprite_defaults();
    }

    int gid = layer->getTileGID(map_x, map_y);
    if (gid == 0)
    {
        return cf_sprite_defaults(); // Empty tile
    }

    auto tileset = findTilesetForGID(gid);
    if (!tileset)
    {
        printf("No tileset found for GID: %d\n", gid);
        return cf_sprite_defaults();
    }

    return tileset->getSpriteForGID(gid);
}

void tmx::renderLayer(int layer_index, float world_x, float world_y) const
{
    auto layer = getLayer(layer_index);
    if (!layer || !layer->visible)
    {
        return;
    }

    printf("Rendering layer %d: '%s' with %dx%d tiles at world position (%.1f, %.1f)\n",
           layer_index, layer->name.c_str(), layer->width, layer->height, world_x, world_y);

    for (int y = 0; y < layer->height; y++)
    {
        for (int x = 0; x < layer->width; x++)
        {
            int gid = layer->getTileGID(x, y);
            if (gid == 0)
                continue; // Skip empty tiles

            auto tileset = findTilesetForGID(gid);
            if (!tileset)
                continue;

            CF_Sprite sprite = tileset->getSpriteForGID(gid);

            // Calculate world position for this tile
            // Coordinate system: (0,0) is top-left, +X goes right, +Y goes down
            float tile_world_x = world_x + (x * tile_width);
            float tile_world_y = world_y + (y * tile_height);

            // Debug: Print first few tile positions to verify coordinate system
            if (x < 3 && y < 3)
            {
                printf("  Tile[%d][%d] GID=%d at world position (%.1f, %.1f)\n",
                       x, y, gid, tile_world_x, tile_world_y);
            }

            // Draw the sprite at the tile position
            cf_draw_push();
            cf_draw_translate_v2(cf_v2(tile_world_x, tile_world_y));
            if (layer->opacity < 1.0f)
            {
                // TODO: Apply opacity if needed
            }
            cf_draw_sprite(&sprite);
            cf_draw_pop();
        }
    }
}

void tmx::renderLayer(const std::string &layer_name, float world_x, float world_y) const
{
    auto layer = getLayer(layer_name);
    if (!layer)
    {
        return;
    }

    // Find layer index and use the index-based render function
    for (size_t i = 0; i < layers.size(); i++)
    {
        if (layers[i] == layer)
        {
            renderLayer(static_cast<int>(i), world_x, world_y);
            return;
        }
    }
}

void tmx::renderAllLayers(float world_x, float world_y) const
{
    for (int i = 0; i < static_cast<int>(layers.size()); i++)
    {
        renderLayer(i, world_x, world_y);
    }
}

void tmx::clearAllSpriteCaches()
{
    printf("Clearing all sprite caches for TMX map '%s'\n", path.c_str());
    for (auto &tileset : tilesets)
    {
        if (tileset)
        {
            tileset->clearCache();
        }
    }
}

void tmx::mapToWorldCoords(int map_x, int map_y, float world_x, float world_y, float &tile_world_x, float &tile_world_y) const
{
    // Coordinate system: (0,0) is top-left, +X goes right, +Y goes down
    tile_world_x = world_x + (map_x * tile_width);
    tile_world_y = world_y + (map_y * tile_height);
}

bool tmx::worldToMapCoords(float world_x, float world_y, float base_world_x, float base_world_y, int &map_x, int &map_y) const
{
    // Convert world coordinates back to map tile coordinates
    float relative_x = world_x - base_world_x;
    float relative_y = world_y - base_world_y;

    map_x = static_cast<int>(relative_x / tile_width);
    map_y = static_cast<int>(relative_y / tile_height);

    // Check if the coordinates are within the map bounds
    return (map_x >= 0 && map_x < map_width && map_y >= 0 && map_y < map_height);
}

// TMXTileset implementation
bool TMXTileset::containsGID(int gid) const
{
    if (!tsx_data || tsx_data->empty())
    {
        return false;
    }

    // For now, assume tilesets have a reasonable upper limit
    // In a real implementation, you'd want to calculate this from the tileset image dimensions
    return gid >= first_gid && gid < first_gid + 1000; // Arbitrary limit for now
}

bool TMXTileset::getLocalTileCoords(int gid, int &tile_x, int &tile_y) const
{
    if (!containsGID(gid) || !tsx_data || tsx_data->empty())
    {
        return false;
    }

    int local_id = gid - first_gid;

    // Calculate tile coordinates based on tileset layout
    // This assumes a standard grid layout
    int tileset_width = tsx_data->getTileWidth();
    if (tileset_width <= 0)
        return false;

    // For now, assume a simple row-based layout
    // In a real implementation, you'd need to know the tileset image dimensions
    int tiles_per_row = 32; // This should be calculated from the tileset image width

    tile_x = local_id % tiles_per_row;
    tile_y = local_id / tiles_per_row;

    return true;
}

CF_Sprite TMXTileset::getSpriteForGID(int gid) const
{
    if (!containsGID(gid) || !tsx_data || tsx_data->empty())
    {
        return cf_sprite_defaults();
    }

    // Check if sprite is already cached
    auto cache_it = sprite_cache.find(gid);
    if (cache_it != sprite_cache.end())
    {
        // Return cached sprite
        return cache_it->second;
    }

    // Not in cache, need to create the sprite
    int tile_x, tile_y;
    if (!getLocalTileCoords(gid, tile_x, tile_y))
    {
        return cf_sprite_defaults();
    }
    // lock tile x and y to 0,0
    // tile_x = 2;
    // tile_y = 1;
    // Create the sprite using TSX
    printf("Creating sprite for GID %d at tile coords (%d, %d)\n", gid, tile_x, tile_y);
    CF_Sprite sprite = tsx_data->getTile(tile_x, tile_y);

    // Cache the sprite for future use
    sprite_cache[gid] = sprite;

    printf("Cached new sprite for GID %d at tile coords (%d, %d)\n", gid, tile_x, tile_y);

    return sprite;
}

void TMXTileset::clearCache()
{
    printf("Clearing sprite cache for tileset '%s' (%zu cached sprites)\n",
           name.c_str(), sprite_cache.size());
    sprite_cache.clear();
}

// TMXLayer implementation
int TMXLayer::getTileGID(int x, int y) const
{
    // Coordinate system: (0,0) is top-left, +X goes right, +Y goes down
    if (!isValidCoordinate(x, y))
    {
        return 0;
    }

    // Calculate linear index: row-major order (y * width + x)
    // This ensures (0,0) is top-left and Y increases downward
    int index = y * width + x;
    if (index >= 0 && index < static_cast<int>(data.size()))
    {
        return data[index];
    }

    return 0;
}

bool TMXLayer::isValidCoordinate(int x, int y) const
{
    // Valid coordinates are within bounds: 0 <= x < width, 0 <= y < height
    return x >= 0 && x < width && y >= 0 && y < height;
}
