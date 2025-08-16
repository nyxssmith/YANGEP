#include <cute.h>
#include <cute_app.h>
#include <cute_draw.h>
#include <cute_input.h>
#include <cute_math.h>
#include "lib/PNGSpriteDemo.h"
#include "lib/DataFile.h"

using namespace Cute;

int main(int argc, char* argv[]) {
    // Initialize Cute Framework
    CF_Result result = make_app("YANGEP - PNG Sprite Demo", 0, 50, 50, 1280, 720, 0, argv[0]);
    if (cf_is_error(result)) {
        printf("Failed to create app: %s\n", result.details ? result.details : "Unknown error");
        return -1;
    }

    // Create demo
    PNGSpriteDemo demo;
    if (!demo.initialize()) {
        printf("Failed to initialize demo\n");
        destroy_app();
        return -1;
    }

    printf("YANGEP started successfully\n");

    // Main game loop
    while (app_is_running()) {
        // Update
        float dt = 0.016f; // Fixed timestep for now
        demo.update(dt);

        // Draw
        app_draw_onto_screen();

        // Clear background
        cf_draw_push_color(make_color(0.1f, 0.1f, 0.1f, 1.0f));
        cf_draw_quad_fill(make_aabb(v2(0, 0), 1280, 720), 0.0f);
        cf_draw_pop_color();

        // Render demo
        demo.render();

        // Render demo information
        demo.renderDemoInfo();

        // Update app
        app_update();
    }

    // Cleanup
    destroy_app();
    printf("YANGEP shutdown complete\n");

    return 0;
}
