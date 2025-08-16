#pragma once

#include <string>
#include <pugixml.hpp>

class tsx : public pugi::xml_document
{
private:
    std::string path;

public:
    tsx() = default;
    tsx(const std::string &path);

    // Parse TSX file from given path
    bool parse(const std::string &path);
};
