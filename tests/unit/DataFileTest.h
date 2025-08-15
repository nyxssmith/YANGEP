#pragma once

#include "fixtures/TestFixture.hpp"
#include "../../src/lib/DataFile.h"
#include <fstream>
#include <nlohmann/json.hpp>

class DataFileTest : public TestFixture {
protected:
    void SetUp() override;
    void TearDown() override;

    void create_test_json_file();
    void cleanup_test_files();
};
