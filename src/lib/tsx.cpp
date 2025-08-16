#include "tsx.h"
#include <cute.h>
#include <functional>

tsx::tsx(const std::string &path) : path(path)
{
    parse(path);
}

bool tsx::parse(const std::string &path)
{
    printf("Attempting to read TSX file: %s\n", path.c_str());

    // Read the entire file using Cute Framework's VFS
    size_t file_size = 0;
    void *file_data = cf_fs_read_entire_file_to_memory(path.c_str(), &file_size);

    if (file_data == nullptr)
    {
        printf("Failed to read TSX file: %s (file_data is null)\n", path.c_str());
        printf("Check if:\n");
        printf("  1. The file exists at the specified path\n");
        printf("  2. The file system is properly mounted\n");
        printf("  3. The path format is correct (use forward slashes)\n");
        return false;
    }

    if (file_size == 0)
    {
        printf("Failed to read TSX file: %s (file_size is 0)\n", path.c_str());
        cf_free(file_data);
        return false;
    }

    printf("Successfully read %zu bytes from file: %s\n", file_size, path.c_str());

    // Parse the XML data using pugixml
    pugi::xml_parse_result result = load_buffer(file_data, file_size);

    // Free the memory allocated by cf_fs_read_entire_file_to_memory
    cf_free(file_data);

    if (!result)
    {
        printf("XML parsing failed for file '%s': %s at offset %td\n",
               path.c_str(), result.description(), result.offset);
        return false;
    }

    // Store the path after successful parsing
    this->path = path;
    printf("Successfully parsed TSX file: %s\n", path.c_str());

    return true;
}

void tsx::debugPrint() const
{
    printf("\n=== TSX Content Analysis ===\n");
    printf("File: %s\n", path.c_str());

    if (empty())
    {
        printf("TSX document is empty or failed to load\n");
        printf("=== End TSX Content ===\n\n");
        return;
    }

    // Get the root element
    pugi::xml_node root = document_element();
    printf("Root element: %s\n", root.name());

    // Print all attributes of the root
    for (pugi::xml_attribute attr : root.attributes())
    {
        printf("  Attribute: %s = %s\n", attr.name(), attr.value());
    }

    // Recursively print all child elements
    std::function<void(pugi::xml_node, int)> printNode = [&](pugi::xml_node node, int depth)
    {
        std::string indent(depth * 2, ' ');

        // Print element name
        if (node.type() == pugi::node_element)
        {
            printf("%sElement: %s", indent.c_str(), node.name());

            // Print attributes
            for (pugi::xml_attribute attr : node.attributes())
            {
                printf(" [%s=%s]", attr.name(), attr.value());
            }

            // Print text content if any
            if (node.text() && strlen(node.text().get()) > 0)
            {
                printf(" Text: \"%s\"", node.text().get());
            }
            printf("\n");

            // Print children
            for (pugi::xml_node child : node.children())
            {
                printNode(child, depth + 1);
            }
        }
    };

    // Print all children of root
    for (pugi::xml_node child : root.children())
    {
        printNode(child, 1);
    }

    printf("=== End TSX Content ===\n\n");
}

CF_Sprite tsx::getTile(int tile_x, int tile_y) const
{
    // Return default sprite if document is empty
    if (empty())
    {
        printf("TSX document is empty, returning default sprite\n");
        return cf_sprite_defaults();
    }

    // Get tileset properties
    pugi::xml_node root = document_element();
    int tile_width = root.attribute("tilewidth").as_int(32);
    int tile_height = root.attribute("tileheight").as_int(32);

    // Find the image node
    pugi::xml_node image_node = root.child("image");
    if (!image_node)
    {
        printf("No image node found in TSX file\n");
        return cf_sprite_defaults();
    }

    // Get image source path
    std::string image_source = image_node.attribute("source").value();
    if (image_source.empty())
    {
        printf("No image source found in TSX file\n");
        return cf_sprite_defaults();
    }

    // Construct full path to image (relative to TSX file location)
    std::string image_path;
    size_t last_slash = path.find_last_of("/\\");
    if (last_slash != std::string::npos)
    {
        image_path = path.substr(0, last_slash + 1) + image_source;
    }
    else
    {
        image_path = image_source;
    }

    printf("Creating tile sprite from: %s at tile coordinates (%d, %d)\n",
           image_path.c_str(), tile_x, tile_y);

    // Load the tileset sprite
    CF_Result result;
    CF_Sprite tile_sprite = cf_make_easy_sprite_from_png(image_path.c_str(), &result);

    if (Cute::is_error(result))
    {
        printf("Failed to load tileset image: %s\n", image_path.c_str());
        return cf_sprite_defaults();
    }

    // Calculate pixel coordinates for reference
    int pixel_x = tile_x * tile_width;
    int pixel_y = tile_y * tile_height;

    // TODO: Need to implement proper tile cropping using Cute Framework's sprite API
    // For now, return the full tileset - the user can manually crop or we need to find
    // the correct API for creating sub-sprites from a texture atlas

    printf("Created tile sprite at (%d, %d) -> pixel (%d, %d) size (%dx%d)\n",
           tile_x, tile_y, pixel_x, pixel_y, tile_width, tile_height);
    printf("Note: Currently returns full tileset image. Tile cropping needs proper CF API.\n");

    return tile_sprite;
}
