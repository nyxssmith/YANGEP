#include <cute.h>
#include <stdio.h>
#include <cstdlib>
#include <chrono>
#include "lib/AnimatedDataCharacter.h"
#include "lib/Utils.h"

using namespace Cute;

int main(int argc, char *argv[])
{
    printf("=== YANGEP Animation Demo ===\n");
    printf("Starting dedicated animation demo executable...\n");

    // Create a window optimized for animation demo
    int options = CF_APP_OPTIONS_WINDOW_POS_CENTERED_BIT | CF_APP_OPTIONS_RESIZABLE_BIT;
    CF_Result result = make_app("YANGEP - Animation Demo", 0, 0, 0, 800, 600, options, argv[0]);

    if (is_error(result))
    {
        printf("ERROR: Failed to create animation demo window: %s\n", result.details);
        return -1;
    }

    printf("Window created successfully (800x600)\n");

    // Set up VFS for reading assets
    try
    {
        printf("Setting up VFS...\n");
        mount_content_directory_as("/assets");
        printf("VFS mounted successfully\n");
    }
    catch (...)
    {
        printf("ERROR: Failed to mount content directory\n");
        destroy_app();
        return -1;
    }

    // Create and initialize the sprite animation demo
    printf("Initializing AnimatedDataCharacter...\n");

    AnimatedDataCharacter demo;
    if (!demo.init("assets/DataFiles/EntityFiles/skeleton.json"))
    {
        printf("ERROR: Failed to initialize sprite animation demo\n");
        destroy_app();
        return -1;
    }

    printf("Animation demo initialized successfully!\n");
    printf("\n=== CONTROLS ===\n");
    printf("WASD/Arrow Keys: Move character & change direction\n");
    printf("1: Switch to idle animation\n");
    printf("2: Switch to walkcycle animation\n");
    printf("SPACE: Toggle between idle/walkcycle\n");
    printf("R: Reset position to center\n");
    printf("ESC: Exit demo\n");
    printf("\n=== Starting main loop ===\n");

    // Timing and state tracking
    auto start_time = std::chrono::high_resolution_clock::now();
    int frame_count = 0;
    bool user_quit = false;

    // Main loop - dedicated to animation demo only
    while (app_is_running())
    {
        app_update(); // Handle window events

        // Handle ESC key to exit
        if (key_just_pressed(CF_KEY_ESCAPE))
        {
            user_quit = true;
            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
            printf("ESC pressed - USER INITIATED CLEAN EXIT after %.3f seconds (%d frames)\n",
                   duration.count() / 1000.0, frame_count);
            break;
        }

        // FIXED: Use single presentation approach to avoid CF graphics crash
        // Draw background manually without separate screen clear
        CF_Color bg = make_color(0.1f, 0.1f, 0.15f, 1.0f); // Dark blue-gray

        cf_draw_push_color(bg);
        cf_draw_quad_fill(make_aabb(v2(0, 0), (float)cf_app_get_width(), (float)cf_app_get_height()), 0.0f);
        cf_draw_pop_color();

        // Update and render animation demo
        try
        {
            float dt = CF_DELTA_TIME;

            // Handle input first
            demo.handleInput();

            // Update animation state
            demo.update(dt);

            // Render the demo
            demo.render();
        }
        catch (const std::exception &e)
        {
            printf("EXCEPTION in main loop: %s\n", e.what());
            printf("Continuing demo...\n");
        }
        catch (...)
        {
            printf("UNKNOWN EXCEPTION in main loop\n");
            printf("Continuing demo...\n");
        }

        // FIXED: Single presentation call (no parameter = default behavior)
        app_draw_onto_screen();
        frame_count++;

        // Debug: Show progress every 30 frames (roughly every 0.5 seconds at 60fps)
        if (frame_count % 30 == 0)
        {
            auto current_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time);
            printf("Animation Demo: Frame %d completed (%.3f seconds elapsed)\n",
                   frame_count, duration.count() / 1000.0);
        }
    }

    printf("\n=== Animation Demo Shutting Down ===\n");

    // Calculate final runtime stats
    auto end_time = std::chrono::high_resolution_clock::now();
    auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    if (user_quit)
    {
        printf("CLEAN EXIT: User-initiated shutdown via ESC key\n");
        printf("Total runtime: %.3f seconds, %d frames rendered\n",
               total_duration.count() / 1000.0, frame_count);
        printf("Average FPS: %.1f\n", frame_count / (total_duration.count() / 1000.0));
    }
    else
    {
        printf("UNEXPECTED EXIT: Demo ended without user request\n");
        printf("Runtime before exit: %.3f seconds, %d frames rendered\n",
               total_duration.count() / 1000.0, frame_count);
        if (frame_count > 0)
        {
            printf("Average FPS before exit: %.1f\n", frame_count / (total_duration.count() / 1000.0));
        }
        printf("This may indicate a technical issue or crash\n");
    }

    destroy_app();
    printf("Animation demo shutdown complete.\n");

    return user_quit ? 0 : 1;
}
