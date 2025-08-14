#include "DataFile.h"
#include <fstream>

// Constructor with filename
DataFile::DataFile(const std::string &filename) : filename(filename)
{
    load(filename);
}

// Load JSON from file
bool DataFile::load(const std::string &filename)
{
    std::ifstream in(filename);
    if (!in.is_open())
        return false;
    try
    {
        in >> *this;
    }
    catch (...)
    {
        return false;
    }
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