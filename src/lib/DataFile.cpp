#include "DataFile.h"
#include <fstream>
#include <cute.h>

// Constructor with path
DataFile::DataFile(const std::string &path) : path(path)
{
    load(path);
}

// Load JSON from file
bool DataFile::load(const std::string &path)
{
    // Read the entire file using Cute Framework's VFS
    size_t file_size = 0;
    void *file_data = cf_fs_read_entire_file_to_memory(path.c_str(), &file_size);

    if (file_data == nullptr || file_size == 0)
        return false;

    try
    {
        // Convert the raw data to a string and parse as JSON
        std::string json_string(static_cast<char *>(file_data), file_size);
        nlohmann::json::operator=(nlohmann::json::parse(json_string));

        // Store the path after successful loading
        this->path = path;
    }
    catch (const nlohmann::json::parse_error &e)
    {
        // Free memory and return false on parse error
        cf_free(file_data);
        return false;
    }
    catch (...)
    {
        // Free memory and return false on any other error
        cf_free(file_data);
        return false;
    }

    // Free the memory allocated by cf_fs_read_entire_file_to_memory
    cf_free(file_data);
    return true;
}

// Load JSON from stored path
bool DataFile::load()
{
    if (path.empty())
        return false;
    return load(path);
}

// Save JSON to file
bool DataFile::save(const std::string &path) const
{
    try
    {
        // Convert JSON to string
        std::string json_string = this->dump(4); // Pretty print with 4 spaces

        // For writing files, we need to use a relative path from the write directory
        // If the path starts with a virtual mount point like "/assets/", we need to strip it
        std::string write_path = path;
        if (write_path.starts_with("/assets/"))
        {
            write_path = write_path.substr(8); // Remove "/assets/" prefix
        }

        // Write using Cute Framework's VFS
        CF_Result result = cf_fs_write_entire_buffer_to_file(write_path.c_str(), json_string.c_str(), json_string.size());

        if (!Cute::is_error(result))
        {
            // Update the stored path after successful save
            const_cast<DataFile *>(this)->path = path;
            return true;
        }
        else
        {
            // Print error information for debugging
            printf("DataFile::save() failed for file '%s' (write path: '%s'). Error occurred.\n", path.c_str(), write_path.c_str());
            printf("Make sure the write directory is set with fs_set_write_directory() before attempting to save.\n");
        }

        return false;
    }
    catch (...)
    {
        printf("DataFile::save() caught exception for file '%s'\n", path.c_str());
        return false;
    }
}

// Save JSON to stored path
bool DataFile::save() const
{
    if (path.empty())
        return false;
    return save(path);
}

// Get path
const std::string &DataFile::getpath() const
{
    return path;
}

// Set path
void DataFile::setpath(const std::string &path)
{
    this->path = path;
}