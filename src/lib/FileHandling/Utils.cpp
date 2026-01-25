#include "Utils.h"
#include "DataFile.h"
#include <cute.h>
#include <fstream>
#include <cstring>
// collection of utility functions

// #include <cute_file_system.h>
using namespace Cute;

void mount_content_directory_as(const char *dir)
{
    CF_Path path = fs_get_base_directory();
    path.normalize();
    path += "/assets";

    // Mount the assets directory for reading
    CF_Result mount_result = fs_mount(path.c_str(), dir);
    if (Cute::is_error(mount_result))
    {
        printf("Failed to mount assets directory '%s' as '%s'\n", path.c_str(), dir);
        return;
    }

    // Set the assets directory as the write directory to enable writing
    CF_Result write_result = fs_set_write_directory(path.c_str());
    if (Cute::is_error(write_result))
    {
        printf("Failed to set write directory to '%s'\n", path.c_str());
        return;
    }

    printf("Successfully mounted '%s' as '%s' and set as write directory\n", path.c_str(), dir);
}

nlohmann::json ReadJson(const std::string &file_path)
{
    nlohmann::json json_obj;

    // Read the entire file using Cute Framework's VFS
    size_t file_size = 0;
    void *file_data = cf_fs_read_entire_file_to_memory(file_path.c_str(), &file_size);

    if (file_data != nullptr && file_size > 0)
    {
        try
        {
            // Convert the raw data to a string and parse as JSON
            std::string json_string(static_cast<char *>(file_data), file_size);
            json_obj = nlohmann::json::parse(json_string);
        }
        catch (const nlohmann::json::parse_error &e)
        {
            // Handle JSON parsing errors
            // For now, just return an empty JSON object
            // In a real application, you might want to log the error
            json_obj = nlohmann::json{};
        }

        // Free the memory allocated by cf_fs_read_entire_file_to_memory
        // TODO ^
    }

    return json_obj;
}

DataFile ReadDataFile(const std::string &file_path)
{
    DataFile df(file_path);
    if (!df.load())
    {
        // Handle error (e.g., log it)
    }
    return df;
}