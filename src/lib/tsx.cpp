#include "tsx.h"
#include <cute.h>
#include <functional>
#include <spng.h>
#include <vector>

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

// Helper function to crop a tile from PNG data using libspng
CF_Sprite tsx::cropTileFromPNG(const std::string &image_path, int tile_x, int tile_y, int tile_width, int tile_height) const
{
    printf("Cropping tile (%d, %d) from %s with size (%dx%d)\n",
           tile_x, tile_y, image_path.c_str(), tile_width, tile_height);

    // Read the entire PNG file using Cute Framework's VFS
    size_t file_size = 0;
    void *file_data = cf_fs_read_entire_file_to_memory(image_path.c_str(), &file_size);

    if (file_data == nullptr)
    {
        printf("Failed to read PNG file: %s\n", image_path.c_str());
        return cf_sprite_defaults();
    }

    // Initialize libspng context
    spng_ctx *ctx = spng_ctx_new(0);
    if (ctx == nullptr)
    {
        printf("Failed to create spng context\n");
        cf_free(file_data);
        return cf_sprite_defaults();
    }

    // Set PNG data
    int ret = spng_set_png_buffer(ctx, file_data, file_size);
    if (ret != 0)
    {
        printf("spng_set_png_buffer error: %s\n", spng_strerror(ret));
        spng_ctx_free(ctx);
        cf_free(file_data);
        return cf_sprite_defaults();
    }

    // Get image header
    struct spng_ihdr ihdr;
    ret = spng_get_ihdr(ctx, &ihdr);
    if (ret != 0)
    {
        printf("spng_get_ihdr error: %s\n", spng_strerror(ret));
        spng_ctx_free(ctx);
        cf_free(file_data);
        return cf_sprite_defaults();
    }

    printf("Original image size: %dx%d, bit depth: %d, color type: %d\n",
           ihdr.width, ihdr.height, ihdr.bit_depth, ihdr.color_type);

    // Calculate tile coordinates in pixels
    int pixel_x = tile_x * tile_width;
    int pixel_y = tile_y * tile_height;

    // Validate tile bounds
    if (pixel_x + tile_width > (int)ihdr.width || pixel_y + tile_height > (int)ihdr.height)
    {
        printf("Tile bounds exceed image dimensions. Tile: (%d,%d)+(%dx%d), Image: %dx%d\n",
               pixel_x, pixel_y, tile_width, tile_height, ihdr.width, ihdr.height);
        spng_ctx_free(ctx);
        cf_free(file_data);
        return cf_sprite_defaults();
    }

    // Decode to RGBA8
    size_t image_size;
    ret = spng_decoded_image_size(ctx, SPNG_FMT_RGBA8, &image_size);
    if (ret != 0)
    {
        printf("spng_decoded_image_size error: %s\n", spng_strerror(ret));
        spng_ctx_free(ctx);
        cf_free(file_data);
        return cf_sprite_defaults();
    }

    // Allocate buffer for the full image
    std::vector<uint8_t> full_image(image_size);
    ret = spng_decode_image(ctx, full_image.data(), image_size, SPNG_FMT_RGBA8, 0);
    if (ret != 0)
    {
        printf("spng_decode_image error: %s\n", spng_strerror(ret));
        spng_ctx_free(ctx);
        cf_free(file_data);
        return cf_sprite_defaults();
    }

    // Clean up libspng resources
    spng_ctx_free(ctx);
    cf_free(file_data);

    // Create buffer for cropped tile (RGBA format)
    std::vector<CF_Pixel> tile_pixels(tile_width * tile_height);

    // Copy the tile region from the full image
    for (int y = 0; y < tile_height; y++)
    {
        for (int x = 0; x < tile_width; x++)
        {
            int src_x = pixel_x + x;
            int src_y = pixel_y + y;
            int src_index = (src_y * ihdr.width + src_x) * 4; // 4 bytes per pixel (RGBA)
            int dst_index = y * tile_width + x;

            // Copy RGBA data
            tile_pixels[dst_index].colors.r = full_image[src_index + 0];
            tile_pixels[dst_index].colors.g = full_image[src_index + 1];
            tile_pixels[dst_index].colors.b = full_image[src_index + 2];
            tile_pixels[dst_index].colors.a = full_image[src_index + 3];
        }
    }

    // Create sprite from pixel data using Cute Framework
    CF_Sprite tile_sprite = cf_make_easy_sprite_from_pixels(tile_pixels.data(), tile_width, tile_height);

    printf("Successfully cropped tile (%d, %d) -> pixel region (%d, %d) size (%dx%d)\n",
           tile_x, tile_y, pixel_x, pixel_y, tile_width, tile_height);

    return tile_sprite;
}

CF_Sprite tsx::getTile(int tile_x, int tile_y) const
{
    // x and y coords are from top left is 0,0 and higher y is down, higher x is right
    //  Return default sprite if document is empty
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

    // Use the new PNG cropping function
    CF_Sprite tile_sprite = cropTileFromPNG(image_path, tile_x, tile_y, tile_width, tile_height);

    return tile_sprite;
}

int tsx::getTileWidth() const
{
    if (empty())
    {
        return 32; // Default tile width
    }

    pugi::xml_node root = document_element();
    return root.attribute("tilewidth").as_int(32);
}

int tsx::getTileHeight() const
{
    if (empty())
    {
        return 32; // Default tile height
    }

    pugi::xml_node root = document_element();
    return root.attribute("tileheight").as_int(32);
}

int tsx::getSourceWidth() const
{
    if (empty())
    {
        return 0; // Default when no document is loaded
    }

    // Find the image node
    pugi::xml_node root = document_element();
    pugi::xml_node image_node = root.child("image");
    if (!image_node)
    {
        return 0; // No image node found
    }

    // Get image source path
    std::string image_source = image_node.attribute("source").value();
    if (image_source.empty())
    {
        return 0; // No image source found
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

    // Read the PNG file and get dimensions using libspng
    size_t file_size = 0;
    void *file_data = cf_fs_read_entire_file_to_memory(image_path.c_str(), &file_size);

    if (file_data == nullptr)
    {
        return 0; // Failed to read file
    }

    // Initialize libspng context
    spng_ctx *ctx = spng_ctx_new(0);
    if (ctx == nullptr)
    {
        cf_free(file_data);
        return 0; // Failed to create context
    }

    // Set PNG data
    int ret = spng_set_png_buffer(ctx, file_data, file_size);
    if (ret != 0)
    {
        spng_ctx_free(ctx);
        cf_free(file_data);
        return 0; // Failed to set PNG buffer
    }

    // Get image header
    struct spng_ihdr ihdr;
    ret = spng_get_ihdr(ctx, &ihdr);
    if (ret != 0)
    {
        spng_ctx_free(ctx);
        cf_free(file_data);
        return 0; // Failed to get header
    }

    // Clean up resources
    spng_ctx_free(ctx);
    cf_free(file_data);

    return static_cast<int>(ihdr.width);
}

int tsx::getSourceHeight() const
{
    if (empty())
    {
        return 0; // Default when no document is loaded
    }

    // Find the image node
    pugi::xml_node root = document_element();
    pugi::xml_node image_node = root.child("image");
    if (!image_node)
    {
        return 0; // No image node found
    }

    // Get image source path
    std::string image_source = image_node.attribute("source").value();
    if (image_source.empty())
    {
        return 0; // No image source found
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

    // Read the PNG file and get dimensions using libspng
    size_t file_size = 0;
    void *file_data = cf_fs_read_entire_file_to_memory(image_path.c_str(), &file_size);

    if (file_data == nullptr)
    {
        return 0; // Failed to read file
    }

    // Initialize libspng context
    spng_ctx *ctx = spng_ctx_new(0);
    if (ctx == nullptr)
    {
        cf_free(file_data);
        return 0; // Failed to create context
    }

    // Set PNG data
    int ret = spng_set_png_buffer(ctx, file_data, file_size);
    if (ret != 0)
    {
        spng_ctx_free(ctx);
        cf_free(file_data);
        return 0; // Failed to set PNG buffer
    }

    // Get image header
    struct spng_ihdr ihdr;
    ret = spng_get_ihdr(ctx, &ihdr);
    if (ret != 0)
    {
        spng_ctx_free(ctx);
        cf_free(file_data);
        return 0; // Failed to get header
    }

    // Clean up resources
    spng_ctx_free(ctx);
    cf_free(file_data);

    return static_cast<int>(ihdr.height);
}
