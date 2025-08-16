#include <cute.h>
#include <cute_png_cache.h>
#include <cstdlib>
#include <cstdio>

using namespace Cute;

// Simple runtime test to demonstrate the CF_ASSERT failure
// This will show us exactly what happens when we try to render virtual PNGs

int main() {
    printf("=== PNG Sprite Runtime Test ===\n");
    printf("This test will demonstrate the CF_ASSERT failure when rendering virtual PNGs\n\n");
    
    // Initialize Cute Framework
    CF_Result result = make_app("Runtime Test", 0, 0, 0, 800, 600, 0, nullptr);
    if (cf_is_error(result)) {
        printf("Failed to initialize Cute Framework: %s\n", 
               result.details ? result.details : "Unknown error");
        return 1;
    }
    
    printf("Cute Framework initialized successfully\n");
    
    // Load the idle sprite sheet
    printf("\n--- Loading Idle Sprite Sheet ---\n");
    CF_Png idlePng;
    result = cf_png_cache_load("assets/Art/AnimationsSheets/idle/BODY_skeleton.png", &idlePng);
    if (cf_is_error(result)) {
        printf("FATAL ERROR: Failed to load idle sprite sheet: %s\n", 
               result.details ? result.details : "Unknown error");
        exit(1);
    }
    printf("Successfully loaded idle sprite sheet: %dx%d\n", idlePng.w, idlePng.h);
    
    // Create a virtual PNG pointing to the DOWN direction (bottom quarter)
    printf("\n--- Creating Virtual PNG for DOWN Direction ---\n");
    CF_Png downFrame;
    downFrame.path = idlePng.path;  // Share the same path
    downFrame.id = idlePng.id;      // Share the same ID
    downFrame.w = 64;               // Frame width
    downFrame.h = 64;               // Frame height
    
    // Point to the DOWN direction region (bottom quarter of 64x256)
    // DOWN is at y=192 (3 * 64), so we offset by 192 rows
    downFrame.pix = idlePng.pix + (192 * idlePng.w);
    
    printf("Created virtual PNG: DOWN direction, size %dx%d\n", downFrame.w, downFrame.h);
    printf("Virtual PNG path: %s\n", downFrame.path ? downFrame.path : "NULL");
    printf("Virtual PNG ID: %llu\n", (unsigned long long)downFrame.id);
    
    // Try to create an animation with this virtual PNG
    printf("\n--- Creating Animation with Virtual PNG ---\n");
    float frameDelay = 1.0f;  // 1 second delay
    const CF_Animation* downAnimation = cf_make_png_cache_animation(
        "idle_down", &downFrame, 1, &frameDelay, 1
    );
    
    if (!downAnimation) {
        printf("ERROR: Failed to create animation with virtual PNG\n");
        printf("This suggests the virtual PNG is invalid even at creation time\n");
    } else {
        printf("SUCCESS: Animation created with virtual PNG\n");
        printf("Animation name: %s\n", downAnimation->name);
        printf("Animation created successfully\n");
        
        // Try to create a sprite with this animation
        printf("\n--- Creating Sprite with Virtual PNG Animation ---\n");
        const CF_Animation* animations[] = {downAnimation};
        const htbl CF_Animation** animationTable = cf_make_png_cache_animation_table(
            "idle_test", animations, 1
        );
        
        if (!animationTable) {
            printf("ERROR: Failed to create animation table\n");
        } else {
            printf("SUCCESS: Animation table created\n");
            
            CF_Sprite sprite = cf_make_png_cache_sprite("idle_test", animationTable);
            printf("SUCCESS: Sprite created with virtual PNG animation\n");
            printf("Sprite dimensions: %dx%d\n", sprite.w, sprite.h);
            
            // Now try to play the animation - this is where CF_ASSERT should fail
            printf("\n--- Attempting to Play Animation (CF_ASSERT Expected) ---\n");
            printf("This should trigger CF_ASSERT(png.path) failure...\n");
            
            cf_sprite_play(&sprite, "idle_down");
            printf("SUCCESS: cf_sprite_play completed without CF_ASSERT failure\n");
            
            // Try to update the sprite - this might also trigger CF_ASSERT
            printf("\n--- Attempting to Update Sprite (CF_ASSERT Expected) ---\n");
            cf_sprite_update(&sprite);
            printf("SUCCESS: cf_sprite_update completed without CF_ASSERT failure\n");
            
            // Try to render the sprite - this is most likely to trigger CF_ASSERT
            printf("\n--- Attempting to Render Sprite (CF_ASSERT Expected) ---\n");
            printf("This is the most likely place for CF_ASSERT(png.path) to fail...\n");
            
            // We need to set up the rendering context first
            cf_draw_push_color(make_color(1.0f, 1.0f, 1.0f, 1.0f));
            cf_draw_sprite(&sprite);
            cf_draw_pop_color();
            
            printf("SUCCESS: cf_draw_sprite completed without CF_ASSERT failure\n");
            printf("This is unexpected - the virtual PNG should have failed by now\n");
        }
    }
    
    printf("\n--- Test Complete ---\n");
    printf("If we got here without CF_ASSERT failures, our virtual PNG approach might actually work!\n");
    printf("But more likely, the failures will happen during actual rendering in the main loop\n");
    
    // Clean up
    cf_png_cache_unload(idlePng);
    destroy_app();
    
    return 0;
}
