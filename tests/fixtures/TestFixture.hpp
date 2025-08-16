#pragma once
#include <gtest/gtest.h>
#include <cute.h>
#include <string>

class TestFixture : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Common setup for all tests
        mount_test_assets();
    }

    void TearDown() override
    {
        // Common cleanup for all tests
    }

    void mount_test_assets()
    {
        // Mount test assets directory
        Cute::CF_Path path = Cute::fs_get_base_directory();
        path.normalize();
        path += "/tests/assets";
        Cute::fs_mount(path.c_str(), "/test_assets");
    }

    std::string get_test_asset_path(const std::string &path)
    {
        return "/test_assets/" + path;
    }
};
