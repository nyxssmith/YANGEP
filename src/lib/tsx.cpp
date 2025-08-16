#include "tsx.h"
#include <cute.h>

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
