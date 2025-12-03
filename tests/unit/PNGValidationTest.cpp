#include <gtest/gtest.h>
#include <cute.h>
#include <spng.h>
#include "Utils.h"
#include "../fixtures/TestFixture.hpp"
#include <vector>
#include <string>

class PNGValidationTest : public TestFixture
{
protected:
    void SetUp() override
    {
        TestFixture::SetUp();
        mount_content_directory_as("/assets");
    }

    // Helper to validate PNG file properties
    struct PNGProperties
    {
        uint32_t width;
        uint32_t height;
        uint8_t bit_depth;
        uint8_t color_type;
        bool valid;
        size_t file_size;

        PNGProperties() : width(0), height(0), bit_depth(0), color_type(0), valid(false), file_size(0) {}
    };

    PNGProperties loadAndValidatePNG(const std::string &path)
    {
        PNGProperties props;

        printf("=== VALIDATING PNG: %s ===\n", path.c_str());

        // Load file
        size_t file_size = 0;
        void *file_data = cf_fs_read_entire_file_to_memory(path.c_str(), &file_size);
        if (!file_data)
        {
            printf("❌ FAILED: Could not read PNG file\n");
            return props;
        }

        props.file_size = file_size;
        printf("✅ File loaded: %zu bytes\n", file_size);

        // Initialize spng
        spng_ctx *ctx = spng_ctx_new(0);
        if (!ctx)
        {
            printf("❌ FAILED: Could not create spng context\n");
            cf_free(file_data);
            return props;
        }

        // Set PNG buffer
        int ret = spng_set_png_buffer(ctx, file_data, file_size);
        if (ret != 0)
        {
            printf("❌ FAILED: spng_set_png_buffer error: %d\n", ret);
            spng_ctx_free(ctx);
            cf_free(file_data);
            return props;
        }

        // Get header
        struct spng_ihdr ihdr;
        ret = spng_get_ihdr(ctx, &ihdr);
        if (ret != 0)
        {
            printf("❌ FAILED: spng_get_ihdr error: %d\n", ret);
            spng_ctx_free(ctx);
            cf_free(file_data);
            return props;
        }

        props.width = ihdr.width;
        props.height = ihdr.height;
        props.bit_depth = ihdr.bit_depth;
        props.color_type = ihdr.color_type;
        props.valid = true;

        printf("✅ Dimensions: %dx%d\n", props.width, props.height);
        printf("✅ Bit depth: %d\n", props.bit_depth);
        printf("✅ Color type: %d\n", props.color_type);

        // Test image decoding
        size_t image_size;
        ret = spng_decoded_image_size(ctx, SPNG_FMT_RGBA8, &image_size);
        if (ret != 0)
        {
            printf("❌ WARNING: Could not get decoded image size: %d\n", ret);
        }
        else
        {
            printf("✅ Expected decoded size: %zu bytes\n", image_size);

            // Validate expected size matches dimensions
            size_t expected_size = props.width * props.height * 4; // RGBA8
            if (image_size == expected_size)
            {
                printf("✅ Size calculation matches: %zu bytes\n", expected_size);
            }
            else
            {
                printf("⚠️  Size mismatch: expected %zu, got %zu\n", expected_size, image_size);
            }
        }

        spng_ctx_free(ctx);
        cf_free(file_data);

        return props;
    }

    // Test extracting a specific tile region
    bool testTileExtraction(const std::string &path, int tile_x, int tile_y, int tile_width, int tile_height)
    {
        printf("=== TESTING TILE EXTRACTION: (%d,%d) %dx%d from %s ===\n",
               tile_x, tile_y, tile_width, tile_height, path.c_str());

        // Load and decode full PNG
        size_t file_size = 0;
        void *file_data = cf_fs_read_entire_file_to_memory(path.c_str(), &file_size);
        if (!file_data)
        {
            printf("❌ FAILED: Could not read PNG file for tile extraction\n");
            return false;
        }

        spng_ctx *ctx = spng_ctx_new(0);
        if (!ctx)
        {
            printf("❌ FAILED: Could not create spng context for tile extraction\n");
            cf_free(file_data);
            return false;
        }

        int ret = spng_set_png_buffer(ctx, file_data, file_size);
        if (ret != 0)
        {
            printf("❌ FAILED: spng_set_png_buffer error in tile extraction: %d\n", ret);
            spng_ctx_free(ctx);
            cf_free(file_data);
            return false;
        }

        struct spng_ihdr ihdr;
        ret = spng_get_ihdr(ctx, &ihdr);
        if (ret != 0)
        {
            printf("❌ FAILED: spng_get_ihdr error in tile extraction: %d\n", ret);
            spng_ctx_free(ctx);
            cf_free(file_data);
            return false;
        }

        // Check bounds
        int pixel_x = tile_x * tile_width;
        int pixel_y = tile_y * tile_height;

        if (pixel_x + tile_width > ihdr.width || pixel_y + tile_height > ihdr.height)
        {
            printf("❌ FAILED: Tile bounds exceed image dimensions\n");
            printf("   Tile region: (%d,%d) to (%d,%d)\n", pixel_x, pixel_y, pixel_x + tile_width, pixel_y + tile_height);
            printf("   Image size: %dx%d\n", ihdr.width, ihdr.height);
            spng_ctx_free(ctx);
            cf_free(file_data);
            return false;
        }

        // Decode full image
        size_t image_size;
        ret = spng_decoded_image_size(ctx, SPNG_FMT_RGBA8, &image_size);
        if (ret != 0)
        {
            printf("❌ FAILED: Could not get image size for tile extraction: %d\n", ret);
            spng_ctx_free(ctx);
            cf_free(file_data);
            return false;
        }

        std::vector<uint8_t> full_image(image_size);
        ret = spng_decode_image(ctx, full_image.data(), image_size, SPNG_FMT_RGBA8, 0);
        if (ret != 0)
        {
            printf("❌ FAILED: Could not decode image for tile extraction: %d\n", ret);
            spng_ctx_free(ctx);
            cf_free(file_data);
            return false;
        }

        // Extract tile pixels
        std::vector<CF_Pixel> tile_pixels(tile_width * tile_height);
        for (int y = 0; y < tile_height; y++)
        {
            for (int x = 0; x < tile_width; x++)
            {
                int src_index = ((pixel_y + y) * ihdr.width + (pixel_x + x)) * 4;
                int dst_index = y * tile_width + x;

                if (src_index + 3 >= full_image.size())
                {
                    printf("❌ FAILED: Source index out of bounds: %d >= %zu\n", src_index + 3, full_image.size());
                    spng_ctx_free(ctx);
                    cf_free(file_data);
                    return false;
                }

                tile_pixels[dst_index].colors.r = full_image[src_index + 0];
                tile_pixels[dst_index].colors.g = full_image[src_index + 1];
                tile_pixels[dst_index].colors.b = full_image[src_index + 2];
                tile_pixels[dst_index].colors.a = full_image[src_index + 3];
            }
        }

        // Test sprite creation from extracted pixels
        CF_Sprite test_sprite = cf_make_easy_sprite_from_pixels(tile_pixels.data(), tile_width, tile_height);

        bool success = (test_sprite.w == tile_width && test_sprite.h == tile_height);

        if (success)
        {
            printf("✅ Tile extraction successful: Created %dx%d sprite\n", test_sprite.w, test_sprite.h);

            // Sample some pixel values to verify extraction
            printf("✅ Sample pixels extracted:\n");
            for (int i = 0; i < std::min(3, tile_width * tile_height); i++)
            {
                printf("   [%d] RGBA(%d,%d,%d,%d)\n", i,
                       tile_pixels[i].colors.r, tile_pixels[i].colors.g,
                       tile_pixels[i].colors.b, tile_pixels[i].colors.a);
            }
        }
        else
        {
            printf("❌ FAILED: Tile extraction created invalid sprite: %dx%d\n", test_sprite.w, test_sprite.h);
        }

        spng_ctx_free(ctx);
        cf_free(file_data);

        return success;
    }
};

// Test that animation PNG files have valid basic properties
TEST_F(PNGValidationTest, AnimationPNGsHaveValidProperties)
{
    // Test the main animation PNG files
    std::vector<std::string> animation_pngs = {
        "assets/Art/AnimationsSheets/walkcycle/BODY_skeleton.png",
        // Add more animation PNGs as they exist
    };

    for (const std::string &png_path : animation_pngs)
    {
        PNGProperties props = loadAndValidatePNG(png_path);

        ASSERT_TRUE(props.valid) << "PNG should be valid: " << png_path;
        EXPECT_GT(props.width, 0) << "PNG width should be positive: " << png_path;
        EXPECT_GT(props.height, 0) << "PNG height should be positive: " << png_path;
        EXPECT_GT(props.file_size, 0) << "PNG file size should be positive: " << png_path;

        // These should match working TMX specifications
        EXPECT_EQ(props.bit_depth, 8) << "PNG bit depth should match working TMX: " << png_path;
        EXPECT_EQ(props.color_type, 6) << "PNG color type should match working TMX: " << png_path;
    }
}

// Test that animation PNGs have expected dimensions for 64x64 tiles
TEST_F(PNGValidationTest, AnimationPNGsHaveExpectedTileDimensions)
{
    PNGProperties props = loadAndValidatePNG("assets/Art/AnimationsSheets/walkcycle/BODY_skeleton.png");

    ASSERT_TRUE(props.valid) << "Animation PNG should be valid";

    // Expected: 9 frames × 64px wide = 576px width
    // Expected: 4 directions × 64px tall = 256px height
    EXPECT_EQ(props.width, 576) << "Animation PNG should be 576px wide (9 × 64px)";
    EXPECT_EQ(props.height, 256) << "Animation PNG should be 256px tall (4 × 64px)";

    // Validate tile grid calculations
    int expected_tiles_per_row = props.width / 64;
    int expected_tile_rows = props.height / 64;

    EXPECT_EQ(expected_tiles_per_row, 9) << "Should have 9 tiles per row";
    EXPECT_EQ(expected_tile_rows, 4) << "Should have 4 tile rows";

    // Total tiles should be 36 (9 × 4)
    int total_tiles = expected_tiles_per_row * expected_tile_rows;
    EXPECT_EQ(total_tiles, 36) << "Should have 36 total tiles";
}

// Compare animation PNG properties against working TMX PNG properties
TEST_F(PNGValidationTest, AnimationPNGsMatchWorkingTMXSpecs)
{
    // Load working TMX PNG
    PNGProperties tmx_props = loadAndValidatePNG("assets/Levels/test_one/tiles/magecity.png");
    ASSERT_TRUE(tmx_props.valid) << "TMX PNG should be valid for comparison";

    // Load animation PNG
    PNGProperties anim_props = loadAndValidatePNG("assets/Art/AnimationsSheets/walkcycle/BODY_skeleton.png");
    ASSERT_TRUE(anim_props.valid) << "Animation PNG should be valid for comparison";

    printf("=== COMPARING TMX vs ANIMATION PNG SPECS ===\n");
    printf("TMX PNG:       %dx%d, bit_depth: %d, color_type: %d, size: %zu bytes\n",
           tmx_props.width, tmx_props.height, tmx_props.bit_depth, tmx_props.color_type, tmx_props.file_size);
    printf("Animation PNG: %dx%d, bit_depth: %d, color_type: %d, size: %zu bytes\n",
           anim_props.width, anim_props.height, anim_props.bit_depth, anim_props.color_type, anim_props.file_size);

    // Critical specs must match for compatibility
    EXPECT_EQ(anim_props.bit_depth, tmx_props.bit_depth) << "Bit depths must match";
    EXPECT_EQ(anim_props.color_type, tmx_props.color_type) << "Color types must match";

    // Both should be valid PNG specifications
    EXPECT_EQ(tmx_props.bit_depth, 8) << "TMX PNG should have 8-bit depth";
    EXPECT_EQ(tmx_props.color_type, 6) << "TMX PNG should have color type 6 (RGBA)";
    EXPECT_EQ(anim_props.bit_depth, 8) << "Animation PNG should have 8-bit depth";
    EXPECT_EQ(anim_props.color_type, 6) << "Animation PNG should have color type 6 (RGBA)";
}

// Test that individual tiles can be extracted from animation PNGs successfully
TEST_F(PNGValidationTest, CanExtractValidTilesFromAnimationPNG)
{
    std::string anim_png = "assets/Art/AnimationsSheets/walkcycle/BODY_skeleton.png";

    // Test extracting tiles from different positions
    struct TileTest
    {
        int tile_x, tile_y;
        std::string description;
    };

    std::vector<TileTest> tile_tests = {
        {0, 0, "Top-left tile (UP direction, frame 0)"},
        {1, 0, "Second tile in top row (UP direction, frame 1)"},
        {8, 0, "Last tile in top row (UP direction, frame 8)"},
        {0, 1, "First tile in second row (LEFT direction, frame 0)"},
        {0, 2, "First tile in third row (DOWN direction, frame 0)"},
        {0, 3, "First tile in bottom row (RIGHT direction, frame 0)"},
        {8, 3, "Bottom-right tile (RIGHT direction, frame 8)"}};

    for (const auto &test : tile_tests)
    {
        printf("\n--- Testing: %s ---\n", test.description.c_str());
        EXPECT_TRUE(testTileExtraction(anim_png, test.tile_x, test.tile_y, 64, 64))
            << "Should be able to extract tile at (" << test.tile_x << "," << test.tile_y << "): " << test.description;
    }
}

// Test that animation PNG pixel data is not corrupted
TEST_F(PNGValidationTest, AnimationPNGPixelDataIntegrity)
{
    std::string anim_png = "assets/Art/AnimationsSheets/walkcycle/BODY_skeleton.png";

    printf("=== TESTING PIXEL DATA INTEGRITY ===\n");

    // Load and fully decode the PNG
    size_t file_size = 0;
    void *file_data = cf_fs_read_entire_file_to_memory(anim_png.c_str(), &file_size);
    ASSERT_NE(file_data, nullptr) << "Should be able to read animation PNG";

    spng_ctx *ctx = spng_ctx_new(0);
    ASSERT_NE(ctx, nullptr) << "Should be able to create spng context";

    ASSERT_EQ(spng_set_png_buffer(ctx, file_data, file_size), 0) << "Should be able to set PNG buffer";

    struct spng_ihdr ihdr;
    ASSERT_EQ(spng_get_ihdr(ctx, &ihdr), 0) << "Should be able to get PNG header";

    // Decode full image
    size_t image_size;
    ASSERT_EQ(spng_decoded_image_size(ctx, SPNG_FMT_RGBA8, &image_size), 0) << "Should be able to get image size";

    std::vector<uint8_t> full_image(image_size);
    ASSERT_EQ(spng_decode_image(ctx, full_image.data(), image_size, SPNG_FMT_RGBA8, 0), 0) << "Should be able to decode full image";

    printf("✅ Successfully decoded %dx%d image (%zu bytes)\n", ihdr.width, ihdr.height, image_size);

    // Verify image data makes sense
    EXPECT_EQ(image_size, ihdr.width * ihdr.height * 4) << "Decoded size should match expected RGBA size";

    // Check for reasonable pixel value distribution
    std::map<uint8_t, int> alpha_distribution;
    std::map<uint8_t, int> red_distribution;
    int transparent_pixels = 0;
    int opaque_pixels = 0;

    for (size_t i = 0; i < image_size; i += 4)
    {
        uint8_t r = full_image[i];
        uint8_t g = full_image[i + 1];
        uint8_t b = full_image[i + 2];
        uint8_t a = full_image[i + 3];

        alpha_distribution[a]++;
        red_distribution[r]++;

        if (a == 0)
            transparent_pixels++;
        else if (a == 255)
            opaque_pixels++;
    }

    int total_pixels = image_size / 4;
    printf("✅ Pixel analysis:\n");
    printf("   Total pixels: %d\n", total_pixels);
    printf("   Transparent pixels (a=0): %d (%.1f%%)\n", transparent_pixels, 100.0f * transparent_pixels / total_pixels);
    printf("   Opaque pixels (a=255): %d (%.1f%%)\n", opaque_pixels, 100.0f * opaque_pixels / total_pixels);
    printf("   Unique alpha values: %zu\n", alpha_distribution.size());
    printf("   Unique red values: %zu\n", red_distribution.size());

    // Reasonable expectations for animation sprites
    EXPECT_GT(transparent_pixels, 0) << "Animation should have some transparent pixels";
    EXPECT_GT(opaque_pixels, 0) << "Animation should have some opaque pixels";
    EXPECT_GE(alpha_distribution.size(), 2) << "Should have at least 2 alpha values";
    EXPECT_GE(red_distribution.size(), 2) << "Should have some color variation";

    spng_ctx_free(ctx);
    cf_free(file_data);
}

// Stress test: Extract many tiles to ensure consistency
TEST_F(PNGValidationTest, AnimationPNGTileExtractionStressTest)
{
    std::string anim_png = "assets/Art/AnimationsSheets/walkcycle/BODY_skeleton.png";

    printf("=== ANIMATION PNG TILE EXTRACTION STRESS TEST ===\n");

    int successful_extractions = 0;
    int failed_extractions = 0;

    // Extract all 36 tiles (9 cols × 4 rows)
    for (int row = 0; row < 4; row++)
    {
        for (int col = 0; col < 9; col++)
        {
            if (testTileExtraction(anim_png, col, row, 64, 64))
            {
                successful_extractions++;
            }
            else
            {
                failed_extractions++;
            }
        }
    }

    printf("=== STRESS TEST RESULTS ===\n");
    printf("Successful extractions: %d\n", successful_extractions);
    printf("Failed extractions: %d\n", failed_extractions);

    EXPECT_EQ(successful_extractions, 36) << "Should be able to extract all 36 tiles";
    EXPECT_EQ(failed_extractions, 0) << "No tile extractions should fail";
}
