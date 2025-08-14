#pragma once
#include <nlohmann/json.hpp>
#include <string>
#include "DataFile.h"
// collection of utility functions

void mount_content_directory_as(const char *dir);

// JSON utility function
nlohmann::json ReadJson(const std::string &file_path);
DataFile ReadDataFile(const std::string &file_path);