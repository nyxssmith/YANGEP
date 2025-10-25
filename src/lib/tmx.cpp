#include "tmx.h"
#include "DataFile.h"
#include "CFNativeCamera.h"
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
            // Convert from TMX coordinate system (0,0 top-left, Y down) to rendering system (Y up)
            float tile_world_x = world_x + (x * tile_width);
            float tile_world_y = world_y + ((layer->height - 1 - y) * tile_height);

            // Round world coordinates to integer pixel coordinates to prevent seams
            // Also round the base world position to ensure alignment
            float rounded_world_x = roundf(world_x);
            float rounded_world_y = roundf(world_y);
            tile_world_x = rounded_world_x + (x * tile_width);
            tile_world_y = rounded_world_y + ((layer->height - 1 - y) * tile_height);

            // Debug: Print first few tile positions to verify coordinate system
            if (x < 3 && y < 3)
            {
                printf("  Tile[%d][%d] GID=%d at world position (%.1f, %.1f)\n",
                       x, y, gid, tile_world_x, tile_world_y);
            }

            // Draw the sprite at the tile position with slight overlap to prevent seams
            cf_draw_push();
            cf_draw_translate_v2(cf_v2(tile_world_x, tile_world_y));

            // Add a tiny scale factor to create overlap between tiles (prevents seams)
            // This makes each tile slightly larger to ensure no gaps appear
            const float overlap_scale = 1.001f; // 0.1% overlap
            cf_draw_scale(overlap_scale, overlap_scale);

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

void tmx::renderLayer(int layer_index, const CFNativeCamera &camera, float world_x, float world_y) const
{
    // Forward to the version with highlight parameter (no highlighting)
    renderLayer(layer_index, camera, false, world_x, world_y);
}

void tmx::renderLayer(int layer_index, const CFNativeCamera &camera, bool highlight_tiles, float world_x, float world_y) const
{
    auto layer = getLayer(layer_index);
    if (!layer || !layer->visible)
    {
        return;
    }

    // Get camera view bounds for culling
    CF_Aabb view_bounds = camera.getViewBounds();

    // printf("Rendering layer %d: '%s' with %dx%d tiles at world position (%.1f, %.1f) with camera culling\n",
    //        layer_index, layer->name.c_str(), layer->width, layer->height, world_x, world_y);

    // Calculate which tiles are potentially visible
    // For X: standard left-to-right calculation
    int start_x = std::max(0, (int)((view_bounds.min.x - world_x) / tile_width) - 1);
    int end_x = std::min(layer->width - 1, (int)((view_bounds.max.x - world_x) / tile_width) + 1);

    // For Y: need to account for coordinate system flip (TMX Y-down vs Rendering Y-up)
    // Convert view bounds from world coordinates (Y up) to TMX layer coordinates (Y down)
    float layer_bottom_world = world_y;                              // Bottom of layer in world coordinates
    float layer_top_world = world_y + (layer->height * tile_height); // Top of layer in world coordinates

    // Map view bounds to TMX layer tile coordinates (flipped Y)
    int start_y_tmx = std::max(0, (int)((layer_top_world - view_bounds.max.y) / tile_height) - 1);
    int end_y_tmx = std::min(layer->height - 1, (int)((layer_top_world - view_bounds.min.y) / tile_height) + 1);

    // printf("  Culling to tiles: x[%d-%d], y[%d-%d] (camera bounds: %.1f,%.1f to %.1f,%.1f)\n",
    //        start_x, end_x, start_y_tmx, end_y_tmx,
    //        view_bounds.min.x, view_bounds.min.y, view_bounds.max.x, view_bounds.max.y);

    int tiles_rendered = 0;
    for (int y = start_y_tmx; y <= end_y_tmx; y++)
    {
        for (int x = start_x; x <= end_x; x++)
        {
            int gid = layer->getTileGID(x, y);
            if (gid == 0)
                continue; // Skip empty tiles

            auto tileset = findTilesetForGID(gid);
            if (!tileset)
                continue;
            // printf("Found tileset '%s' for GID %d\n", tileset->name.c_str(), gid);
            CF_Sprite sprite = tileset->getSpriteForGID(gid);

            // Calculate world position for this tile
            // Convert from TMX coordinate system (0,0 top-left, Y down) to rendering system (Y up)
            float tile_world_x = world_x + (x * tile_width);
            float tile_world_y = world_y + ((layer->height - 1 - y) * tile_height);

            // Round world coordinates to zoom-aware pixel boundaries to prevent seams
            // This ensures tiles align properly at any zoom level
            float camera_zoom = camera.getZoom();
            if (camera_zoom != 1.0f)
            {
                // Round to zoom-adjusted pixel boundaries
                float rounded_world_x = roundf(world_x * camera_zoom) / camera_zoom;
                float rounded_world_y = roundf(world_y * camera_zoom) / camera_zoom;
                tile_world_x = rounded_world_x + (x * tile_width);
                tile_world_y = rounded_world_y + ((layer->height - 1 - y) * tile_height);

                // Also round the final tile position
                tile_world_x = roundf(tile_world_x * camera_zoom) / camera_zoom;
                tile_world_y = roundf(tile_world_y * camera_zoom) / camera_zoom;
            }
            else
            {
                // At 1x zoom, just round to integer pixels
                float rounded_world_x = roundf(world_x);
                float rounded_world_y = roundf(world_y);
                tile_world_x = rounded_world_x + (x * tile_width);
                tile_world_y = rounded_world_y + ((layer->height - 1 - y) * tile_height);
            }

            // Create tile bounds for more precise visibility check
            CF_Aabb tile_bounds = make_aabb(
                cf_v2(tile_world_x, tile_world_y),
                cf_v2(tile_world_x + tile_width, tile_world_y + tile_height));

            // Check if this specific tile is visible
            if (!camera.isVisible(tile_bounds))
                continue;

            // Draw the sprite at the tile position with zoom-dependent overlap to prevent seams
            cf_draw_push();
            cf_draw_translate_v2(cf_v2(tile_world_x, tile_world_y));

            // Calculate overlap based on zoom level to prevent seams at any zoom
            // Higher zoom needs much more overlap to compensate for precision issues
            // Reuse camera_zoom variable from above
            float overlap_scale;
            if (camera_zoom >= 4.0f)
            {
                overlap_scale = 1.05f; // 5% overlap for very high zoom
            }
            else if (camera_zoom >= 2.0f)
            {
                overlap_scale = 1.03f; // 3% overlap for high zoom
            }
            else if (camera_zoom >= 1.5f)
            {
                overlap_scale = 1.015f; // 1.5% overlap for medium zoom
            }
            else
            {
                overlap_scale = 1.01f; // 1% overlap for low zoom (increased from 0.2%)
            }
            cf_draw_scale(overlap_scale, overlap_scale);

            if (layer->opacity < 1.0f)
            {
                // TODO: Apply opacity if needed
            }
            cf_draw_sprite(&sprite);
            cf_draw_pop();

            // Draw tile highlight border if enabled for this layer
            if (highlight_tiles)
            {
                // Sprites are drawn from their center, so adjust the highlight rectangle
                // to match where the sprite actually appears
                float half_width = tile_width / 2.0f;
                float half_height = tile_height / 2.0f;

                CF_Aabb tile_rect;
                tile_rect.min = cf_v2(tile_world_x - half_width, tile_world_y - half_height);
                tile_rect.max = cf_v2(tile_world_x + half_width, tile_world_y + half_height);

                cf_draw_push_color(make_color(1.0f, 1.0f, 0.0f, 0.8f)); // Yellow, slightly transparent
                cf_draw_quad(tile_rect, 0.0f, 2.0f);                    // 2px thick outline
                cf_draw_pop_color();
            }

            tiles_rendered++;
        }
    }

    // printf("  Rendered %d tiles (culled %d tiles)\n", tiles_rendered, layer->width * layer->height - tiles_rendered);
}

void tmx::renderLayer(const std::string &layer_name, const CFNativeCamera &camera, float world_x, float world_y) const
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
            renderLayer(static_cast<int>(i), camera, world_x, world_y);
            return;
        }
    }
}

void tmx::renderAllLayers(const CFNativeCamera &camera, float world_x, float world_y) const
{
    for (int i = 0; i < static_cast<int>(layers.size()); i++)
    {
        renderLayer(i, camera, false, world_x, world_y);
    }
}

void tmx::renderAllLayers(const CFNativeCamera &camera, const DataFile &config, float world_x, float world_y) const
{
    // Render each layer with highlighting based on pre-configured map
    for (int i = 0; i < static_cast<int>(layers.size()); i++)
    {
        bool should_highlight = false;
        if (i < static_cast<int>(layers.size()) && layers[i])
        {
            const std::string &layer_name = layers[i]->name;
            // Check highlight map (efficient O(log n) lookup)
            auto it = layer_highlight_map.find(layer_name);
            if (it != layer_highlight_map.end())
            {
                should_highlight = it->second;
            }
        }
        renderLayer(i, camera, should_highlight, world_x, world_y);
    }

    // After rendering all layers, draw border highlights for configured layers
    for (int i = 0; i < static_cast<int>(layers.size()); i++)
    {
        if (i < static_cast<int>(layers.size()) && layers[i])
        {
            const std::string &layer_name = layers[i]->name;

            // Check if this layer should have border highlighting (cyan)
            auto border_it = layer_border_highlight_map.find(layer_name);
            if (border_it != layer_border_highlight_map.end() && border_it->second)
            {
                // Check if borders are cached for this layer
                auto cache_it = layer_border_cache.find(i);
                if (cache_it == layer_border_cache.end())
                {
                    // Not cached, calculate and cache the border edges
                    layer_border_cache[i] = calculateLayerBorderEdges(i, world_x, world_y);
                    printf("Cached border edges for layer '%s': %zu edges\n",
                           layer_name.c_str(), layer_border_cache[i].size());
                }

                // Draw the cached border edges in cyan
                const auto &edges = layer_border_cache[i];
                cf_draw_push_color(make_color(0.0f, 1.0f, 1.0f, 0.9f)); // Cyan, mostly opaque
                for (const auto &edge : edges)
                {
                    // Only draw if visible in camera
                    if (camera.isVisible(edge))
                    {
                        cf_draw_quad(edge, 0.0f, 3.0f); // 3px thick outline
                    }
                }
                cf_draw_pop_color();
            }

            // Check if this layer should have outer border highlighting (magenta)
            auto outer_border_it = layer_outer_border_highlight_map.find(layer_name);
            if (outer_border_it != layer_outer_border_highlight_map.end() && outer_border_it->second)
            {
                // Check if outer border lines are cached for this layer
                auto cache_it = layer_outer_border_cache.find(i);
                if (cache_it == layer_outer_border_cache.end())
                {
                    // Not cached, calculate and cache the outer border lines
                    layer_outer_border_cache[i] = calculateLayerOuterBorderLines(i, world_x, world_y);
                    printf("Cached outer border lines for layer '%s': %zu lines\n",
                           layer_name.c_str(), layer_outer_border_cache[i].size());
                }

                // Draw the cached outer border lines in magenta
                const auto &lines = layer_outer_border_cache[i];
                cf_draw_push_color(make_color(1.0f, 0.0f, 1.0f, 0.9f)); // Magenta, mostly opaque
                for (const auto &line : lines)
                {
                    // Create a small AABB around the line for visibility check
                    CF_Aabb line_bounds;
                    line_bounds.min = cf_v2(
                        std::min(line.start.x, line.end.x) - 1,
                        std::min(line.start.y, line.end.y) - 1);
                    line_bounds.max = cf_v2(
                        std::max(line.start.x, line.end.x) + 1,
                        std::max(line.start.y, line.end.y) + 1);

                    // Only draw if visible in camera
                    if (camera.isVisible(line_bounds))
                    {
                        cf_draw_line(line.start, line.end, 3.0f); // 3px thick line
                    }
                }
                cf_draw_pop_color();
            }
        }
    }
}

void tmx::setLayerHighlightConfig(const DataFile &config)
{
    // Clear existing configuration
    layer_highlight_map.clear();
    layer_border_highlight_map.clear();
    layer_outer_border_highlight_map.clear();
    layer_border_cache.clear();
    layer_outer_border_cache.clear();

    // Parse highlightLayers from config once
    if (config.contains("Debug") && config["Debug"].contains("highlightLayers"))
    {
        auto &layers_json = config["Debug"]["highlightLayers"];
        if (layers_json.is_array())
        {
            for (const auto &layer_name : layers_json)
            {
                if (layer_name.is_string())
                {
                    std::string name = layer_name.get<std::string>();
                    layer_highlight_map[name] = true;
                }
            }
        }
    }

    // Parse highlightLayerBorders from config once
    if (config.contains("Debug") && config["Debug"].contains("highlightLayerBorders"))
    {
        auto &borders_json = config["Debug"]["highlightLayerBorders"];
        if (borders_json.is_array())
        {
            for (const auto &layer_name : borders_json)
            {
                if (layer_name.is_string())
                {
                    std::string name = layer_name.get<std::string>();
                    layer_border_highlight_map[name] = true;
                }
            }
        }
    }

    // Parse highlightLayerOuterBorders from config once
    if (config.contains("Debug") && config["Debug"].contains("highlightLayerOuterBorders"))
    {
        auto &outer_borders_json = config["Debug"]["highlightLayerOuterBorders"];
        if (outer_borders_json.is_array())
        {
            for (const auto &layer_name : outer_borders_json)
            {
                if (layer_name.is_string())
                {
                    std::string name = layer_name.get<std::string>();
                    layer_outer_border_highlight_map[name] = true;
                }
            }
        }
    }

    printf("Configured layer highlighting for %zu layers\n", layer_highlight_map.size());
    printf("Configured layer border highlighting for %zu layers\n", layer_border_highlight_map.size());
    printf("Configured layer outer border highlighting for %zu layers\n", layer_outer_border_highlight_map.size());
}

std::vector<CF_Aabb> tmx::calculateLayerBorderEdges(int layer_index, float world_x, float world_y) const
{
    std::vector<CF_Aabb> edges;

    auto layer = getLayer(layer_index);
    if (!layer || !layer->visible)
    {
        return edges;
    }

    // For each tile, check if it's on the border of the filled area
    // A tile is on the border if it's filled and has at least one empty neighbor
    for (int y = 0; y < layer->height; y++)
    {
        for (int x = 0; x < layer->width; x++)
        {
            int gid = layer->getTileGID(x, y);
            if (gid == 0)
                continue; // Skip empty tiles

            // Check if this tile is on the border (has at least one empty neighbor or edge)
            bool is_border = false;

            // Check all 4 directions (up, down, left, right)
            // Top
            if (y == 0 || layer->getTileGID(x, y - 1) == 0)
                is_border = true;
            // Bottom
            if (y == layer->height - 1 || layer->getTileGID(x, y + 1) == 0)
                is_border = true;
            // Left
            if (x == 0 || layer->getTileGID(x - 1, y) == 0)
                is_border = true;
            // Right
            if (x == layer->width - 1 || layer->getTileGID(x + 1, y) == 0)
                is_border = true;

            if (is_border)
            {
                // Calculate world position for this border tile
                float tile_world_x = world_x + (x * tile_width);
                float tile_world_y = world_y + ((layer->height - 1 - y) * tile_height);

                // Create AABB for this tile (centered, like the sprites)
                float half_width = tile_width / 2.0f;
                float half_height = tile_height / 2.0f;

                CF_Aabb tile_rect;
                tile_rect.min = cf_v2(tile_world_x - half_width, tile_world_y - half_height);
                tile_rect.max = cf_v2(tile_world_x + half_width, tile_world_y + half_height);

                edges.push_back(tile_rect);
            }
        }
    }

    return edges;
}

std::vector<EdgeLine> tmx::calculateLayerOuterBorderLines(int layer_index, float world_x, float world_y) const
{
    std::vector<EdgeLine> lines;

    auto layer = getLayer(layer_index);
    if (!layer || !layer->visible)
    {
        return lines;
    }

    // For each tile, check which edges face empty space (are outer edges)
    for (int y = 0; y < layer->height; y++)
    {
        for (int x = 0; x < layer->width; x++)
        {
            int gid = layer->getTileGID(x, y);
            if (gid == 0)
                continue; // Skip empty tiles

            // Calculate world position for this tile
            float tile_world_x = world_x + (x * tile_width);
            float tile_world_y = world_y + ((layer->height - 1 - y) * tile_height);

            // Calculate tile boundaries (centered, like the sprites)
            float half_width = tile_width / 2.0f;
            float half_height = tile_height / 2.0f;

            float left = tile_world_x - half_width;
            float right = tile_world_x + half_width;
            float bottom = tile_world_y - half_height;
            float top = tile_world_y + half_height;

            // Check each edge and add a line if it faces empty space

            // Top edge (faces up)
            if (y == 0 || layer->getTileGID(x, y - 1) == 0)
            {
                EdgeLine line;
                line.start = cf_v2(left, top);
                line.end = cf_v2(right, top);
                lines.push_back(line);
            }

            // Bottom edge (faces down)
            if (y == layer->height - 1 || layer->getTileGID(x, y + 1) == 0)
            {
                EdgeLine line;
                line.start = cf_v2(left, bottom);
                line.end = cf_v2(right, bottom);
                lines.push_back(line);
            }

            // Left edge (faces left)
            if (x == 0 || layer->getTileGID(x - 1, y) == 0)
            {
                EdgeLine line;
                line.start = cf_v2(left, bottom);
                line.end = cf_v2(left, top);
                lines.push_back(line);
            }

            // Right edge (faces right)
            if (x == layer->width - 1 || layer->getTileGID(x + 1, y) == 0)
            {
                EdgeLine line;
                line.start = cf_v2(right, bottom);
                line.end = cf_v2(right, top);
                lines.push_back(line);
            }
        }
    }

    return lines;
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
    // Convert from TMX coordinate system (0,0 top-left, Y down) to rendering system (Y up)
    tile_world_x = world_x + (map_x * tile_width);
    tile_world_y = world_y + ((map_height - 1 - map_y) * tile_height);
}

bool tmx::worldToMapCoords(float world_x, float world_y, float base_world_x, float base_world_y, int &map_x, int &map_y) const
{
    // Convert world coordinates back to map tile coordinates
    float relative_x = world_x - base_world_x;
    float relative_y = world_y - base_world_y;

    map_x = static_cast<int>(relative_x / tile_width);

    // Convert from rendering system (Y up) back to TMX coordinate system (Y down)
    int rendered_map_y = static_cast<int>(relative_y / tile_height);
    map_y = map_height - 1 - rendered_map_y;

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
    int source_width = tsx_data->getSourceWidth();
    if (tileset_width <= 0)
        return false;

    int tiles_per_row = source_width / tileset_width;

    tile_x = local_id % tiles_per_row;
    tile_y = local_id / tiles_per_row;
    printf("Local tile coords for GID %d: (%d, %d)\n", gid, tile_x, tile_y);
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
    printf("Creating sprite for GID %d\n", gid);
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
