#include "DataFile.h"
#include <fstream>
#include <cute.h>

// Constructor with filename
DataFile::DataFile(const std::string &filename) : filename(filename)
{
    load(filename);
}

// Load JSON from file
bool DataFile::load(const std::string &filename)
{
    // Read the entire file using Cute Framework's VFS
    size_t file_size = 0;
    void *file_data = cf_fs_read_entire_file_to_memory(filename.c_str(), &file_size);

    if (file_data == nullptr || file_size == 0)
        return false;

    try
    {
        // Convert the raw data to a string and parse as JSON
        std::string json_string(static_cast<char *>(file_data), file_size);
        nlohmann::json::operator=(nlohmann::json::parse(json_string));

        // Store the filename after successful loading
        this->filename = filename;
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

// Load JSON from stored filename
bool DataFile::load()
{
    if (filename.empty())
        return false;
    return load(filename);
}

// Save JSON to file
bool DataFile::save(const std::string &filename) const
{
    std::ofstream out(filename);
    if (!out.is_open())
        return false;
    try
    {
        out << *this;
    }
    catch (...)
    {
        return false;
    }
    return true;
}

// Save JSON to stored filename
bool DataFile::save() const
{
    if (filename.empty())
        return false;
    return save(filename);
}

// Get filename
const std::string &DataFile::getFilename() const
{
    return filename;
}

// Set filename
void DataFile::setFilename(const std::string &filename)
{
    this->filename = filename;
}