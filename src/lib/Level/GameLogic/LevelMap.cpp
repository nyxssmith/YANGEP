#include "LevelMap.h"
#include "DataFile.h"
#include "CFNativeCamera.h"
#include <cstdio>
#include <algorithm>
#include <cctype>
#include <sstream>

LevelMap::LevelMap(const std::string &path)
    : tmx() // Call default constructor, not the one that calls parse()
{
    // Manually call parse to ensure our loadLayers() override gets called
    // Note: parse() is called here after LevelMap is fully constructed,
    // so the virtual function table will correctly point to LevelMap::loadLayers()
    parse(path);
}

bool LevelMap::loadLayers()
{
    // Complete override - does NOT call base class implementation
    // Handles structure layers, navmesh layers, and regular layers

    pugi::xml_node map_node = document_element();
    int regular_layer_count = 0;
    int navmesh_layer_count = 0;
    int structure_layer_count = 0;

    for (pugi::xml_node layer_node : map_node.children("layer"))
    {
        auto layer = std::make_shared<TMXLayer>();

        layer->id = layer_node.attribute("id").as_int(0);
        layer->name = layer_node.attribute("name").value();
        layer->width = layer_node.attribute("width").as_int(getMapWidth());
        layer->height = layer_node.attribute("height").as_int(getMapHeight());
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
            // Parse CSV data inline
            std::string csv_data = data_node.text().get();
            layer->data.clear();

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
                        layer->data.push_back(tile_id);
                    }
                }
            }
        }
        else
        {
            printf("Warning: Unsupported encoding '%s' for layer '%s'\n",
                   encoding.c_str(), layer->name.c_str());
            continue;
        }

        printf("Layer '%s' loaded with %d tiles\n",
               layer->name.c_str(), static_cast<int>(layer->data.size()));

        // Determine layer type based on name (case-insensitive)
        std::string layer_name_lower = layer->name;
        std::transform(layer_name_lower.begin(), layer_name_lower.end(), layer_name_lower.begin(), ::tolower);

        // Check if this is a structure layer (starts with "structure" or "structure_")
        bool is_structure_layer = (layer_name_lower.find("structure") == 0 ||
                                   layer_name_lower.find("structure_") == 0 ||
                                   layer_name_lower.find("structure ") == 0);

        if (is_structure_layer)
        {
            // Create a StructureLayer from the TMXLayer and add to structures list
            auto structureLayer = std::make_shared<StructureLayer>(*layer);
            structures.push_back(structureLayer);
            structure_layer_count++;
            printf("  -> Added to structure layers\n");
            continue; // Skip base class layer lists
        }

        // Check if this is a navmesh layer
        bool is_navmesh_layer = (layer_name_lower.find("navmesh") == 0 ||
                                 layer_name_lower.find("nav_") == 0);

        if (is_navmesh_layer)
        {
            navmesh_layers.push_back(layer);
            navmesh_layer_count++;
            printf("  -> Added to navmesh layers\n");
        }
        else
        {
            layers.push_back(layer);
            regular_layer_count++;
            printf("  -> Added to regular layers\n");
        }
    }

    printf("Loaded %d regular layers, %d navmesh layers, and %d structure layers\n",
           regular_layer_count, navmesh_layer_count, structure_layer_count);

    return !layers.empty() || !navmesh_layers.empty() || !structures.empty();
}

StructureLayer::StructureLayer(const TMXLayer &tmxLayer)
    : id(tmxLayer.id),
      name(tmxLayer.name),
      width(tmxLayer.width),
      height(tmxLayer.height),
      visible(tmxLayer.visible),
      opacity(tmxLayer.opacity),
      data(tmxLayer.data),
      lowestWorldYCoordinate(0)
{
    // Store a copy of the TMX layer for rendering
    this->tmxLayer = std::make_shared<TMXLayer>(tmxLayer);
}

int StructureLayer::getTileGID(int x, int y) const
{
    if (!isValidCoordinate(x, y))
    {
        return 0;
    }

    int index = y * width + x;
    return data[index];
}

bool StructureLayer::isValidCoordinate(int x, int y) const
{
    return x >= 0 && x < width && y >= 0 && y < height;
}

std::shared_ptr<StructureLayer> LevelMap::getStructure(int index) const
{
    if (index < 0 || index >= static_cast<int>(structures.size()))
    {
        return nullptr;
    }
    return structures[index];
}

void LevelMap::addStructure(std::shared_ptr<StructureLayer> structure)
{
    structures.push_back(structure);
}

void LevelMap::renderSingleLayer(std::shared_ptr<TMXLayer> layer, const CFNativeCamera &camera, const DataFile &config, float worldX, float worldY) const
{
    if (!layer || !layer->visible)
    {
        return;
    }

    // Render the layer directly without searching for it in the layers vector
    // This is necessary for structure layers which aren't in the base class layers list

    // Get camera view bounds for culling
    CF_Aabb view_bounds = camera.getViewBounds();

    // Calculate which tiles are potentially visible
    int start_x = std::max(0, (int)((view_bounds.min.x - worldX) / getTileWidth()) - 1);
    int end_x = std::min(layer->width - 1, (int)((view_bounds.max.x - worldX) / getTileWidth()) + 1);

    // For Y: account for coordinate system flip (TMX Y-down vs Rendering Y-up)
    float layer_bottom_world = worldY;
    float layer_top_world = worldY + (layer->height * getTileHeight());

    int start_y_tmx = std::max(0, (int)((layer_top_world - view_bounds.max.y) / getTileHeight()) - 1);
    int end_y_tmx = std::min(layer->height - 1, (int)((layer_top_world - view_bounds.min.y) / getTileHeight()) + 1);

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

            CF_Sprite sprite = tileset->getSpriteForGID(gid);

            // Calculate world position for this tile (convert TMX coords to rendering coords)
            float tile_world_x = worldX + (x * getTileWidth());
            float tile_world_y = worldY + ((layer->height - 1 - y) * getTileHeight());

            // Round world coordinates to zoom-aware pixel boundaries to prevent seams
            float camera_zoom = camera.getZoom();
            if (camera_zoom != 1.0f)
            {
                float rounded_world_x = roundf(worldX * camera_zoom) / camera_zoom;
                float rounded_world_y = roundf(worldY * camera_zoom) / camera_zoom;
                tile_world_x = rounded_world_x + (x * getTileWidth());
                tile_world_y = rounded_world_y + ((layer->height - 1 - y) * getTileHeight());

                tile_world_x = roundf(tile_world_x * camera_zoom) / camera_zoom;
                tile_world_y = roundf(tile_world_y * camera_zoom) / camera_zoom;
            }
            else
            {
                float rounded_world_x = roundf(worldX);
                float rounded_world_y = roundf(worldY);
                tile_world_x = rounded_world_x + (x * getTileWidth());
                tile_world_y = rounded_world_y + ((layer->height - 1 - y) * getTileHeight());
            }

            // Create tile bounds for visibility check
            CF_Aabb tile_bounds = cf_make_aabb(
                cf_v2(tile_world_x, tile_world_y),
                cf_v2(tile_world_x + getTileWidth(), tile_world_y + getTileHeight()));

            if (!camera.isVisible(tile_bounds))
                continue;

            // Draw the sprite
            cf_draw_push();
            cf_draw_translate_v2(cf_v2(tile_world_x, tile_world_y));

            // Calculate overlap based on zoom level to prevent seams
            float overlap_scale;
            if (camera_zoom >= 4.0f)
            {
                overlap_scale = 1.05f;
            }
            else if (camera_zoom >= 2.0f)
            {
                overlap_scale = 1.03f;
            }
            else if (camera_zoom >= 1.5f)
            {
                overlap_scale = 1.015f;
            }
            else
            {
                overlap_scale = 1.01f;
            }
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
