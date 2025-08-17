#pragma once

#include <string>
#include <pugixml.hpp>
#include <cute.h>

class tsx : public pugi::xml_document
{
private:
    std::string path;

    // Helper function to crop a tile from PNG data
    CF_Sprite cropTileFromPNG(const std::string &image_path, int tile_x, int tile_y, int tile_width, int tile_height) const;

public:
    tsx() = default;
    tsx(const std::string &path);

    // Parse TSX file from given path
    bool parse(const std::string &path);

    // Debug function to print all TSX content
    void debugPrint() const;

    // Get a tile sprite by tile coordinates (x, y in tile units)
    CF_Sprite getTile(int tile_x, int tile_y) const;

    // Get tile dimensions
    int getTileWidth() const;
    int getTileHeight() const;

    // Get source image dimensions
    int getSourceWidth() const;
    int getSourceHeight() const;
};
