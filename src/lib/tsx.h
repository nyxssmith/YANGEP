#pragma once

#include <string>
#include <pugixml.hpp>
#include <cute.h>

class tsx : public pugi::xml_document
{
private:
    std::string path;

public:
    tsx() = default;
    tsx(const std::string &path);

    // Parse TSX file from given path
    bool parse(const std::string &path);

    // Debug function to print all TSX content
    void debugPrint() const;

    // Get a tile sprite by tile coordinates (x, y in tile units)
    // Note: Currently returns full tileset image, tile cropping to be implemented
    CF_Sprite getTile(int tile_x, int tile_y) const;
};
