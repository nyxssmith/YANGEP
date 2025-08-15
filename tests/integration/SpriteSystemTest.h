#pragma once

#include "fixtures/TestFixture.hpp"
#include "../../src/lib/DataFile.h"
#include "../../src/lib/Utils.h"
#include <fstream>

class SpriteSystemIntegrationTest : public TestFixture {
protected:
    void SetUp() override;
    void TearDown() override;

    void create_test_assets();
    void cleanup_test_assets();
};
