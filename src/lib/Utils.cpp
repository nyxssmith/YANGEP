// collection of utility functions
#include "Utils.h"
#include <cute.h>
// #include <cute_file_system.h>
using namespace Cute;
void mount_content_directory_as(const char *dir)
{
    CF_Path path = fs_get_base_directory();
    path.normalize();
    path.pop(2); // Pop out of build/debug/
    path += "/assets";
    fs_mount(path.c_str(), dir);
}