#include <cute.h>
#include <stdio.h>
#include <stdlib.h>
#include "lib/DataFile.h"
#include "lib/SpriteBatchDemo.h"

using namespace Cute;

int main(int argc, char* argv[])
{
    // Initialize Cute Framework
    CF_Result result = make_app("YANGEP - Yet Another New Game Engine Project", 0, 0, 0, 640, 480, 0, argv[0]);
    if (is_error(result)) {
        printf("Failed to create app\n");
        return -1;
    }

    // Test DataFile functionality
    DataFile dataFile;
    if (dataFile.load("assets/a.json")) {
        printf("JSON data: %s\n", dataFile.dump(4).c_str());
    } else {
        printf("DataFile data: %s\n", dataFile.dump(4).c_str());
    }

    // Initialize SpriteBatchDemo
    SpriteBatchDemo spriteBatchDemo;
    if (!spriteBatchDemo.initialize()) {
        printf("Failed to initialize SpriteBatchDemo\n");
        return -1;
    }

    printf("Entering main loop...\n");

    // Main game loop
    while (app_is_running()) {
        app_update();

        // Clear the screen with dark background
        draw_push_color(make_color(0.1f, 0.1f, 0.1f, 1.0f));
        draw_quad_fill(make_aabb(v2(-320, -240), 640, 480), 0.0f);
        draw_pop_color();

        // Update and render the SpriteBatchDemo
        float dt = 0.016f; // ~60 FPS for now
        spriteBatchDemo.update(dt);
        spriteBatchDemo.render();

        app_draw_onto_screen(true);
    }

    printf("Exited main loop\n");
    destroy_app();
    return 0;
}
