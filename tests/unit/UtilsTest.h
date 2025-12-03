#pragma once

#include "fixtures/TestFixture.hpp"
#include "Utils.h"
#include <nlohmann/json.hpp>

class UtilsTest : public TestFixture
{
protected:
    void SetUp() override;
    void TearDown() override;

    void create_test_json_file();
    void cleanup_test_files();
};
