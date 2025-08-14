#include <cute.h>
#include <stdio.h>
#include <cimgui.h>
#include "lib/DebugWindow.h"
using namespace Cute;

int main(int argc, char *argv[])
{
	// Create a window with a resolution of 640 x 480.
	int options = CF_APP_OPTIONS_WINDOW_POS_CENTERED_BIT;
	CF_Result result = make_app("Fancy Window Title", 0, 0, 0, 640, 480, options, argv[0]);
	cf_app_init_imgui();
	if (is_error(result))
		return -1;

	// Create debug window
	DebugWindow debugWindow("Debug Info aaaa");

	// Set up triangle vertices (equilateral triangle)
	v2 triangle_vertices[3] = {
		v2(0, -50),		// top vertex
		v2(-43.3f, 25), // bottom left vertex
		v2(43.3f, 25)	// bottom right vertex
	};

	// Triangle positions (3 triangles in different positions)
	v2 triangle_positions[3] = {
		v2(-150, 0), // left triangle
		v2(0, 0),	 // center triangle
		v2(150, 0)	 // right triangle
	};

	// Rotation variables
	float rotation = 0.0f;
	float rotation_speed = 90.0f; // degrees per second

	while (app_is_running())
	{
		app_update();

		// Render debug window
		debugWindow.render();

		// Update rotation
		float dt = CF_DELTA_TIME;
		rotation += rotation_speed * dt;

		// Clear the screen
		draw_quad_fill(make_aabb(v2(0, 0), 640, 480), 0);

		// Set up orthographic projection centered on screen
		// This creates a coordinate system where (0,0) is at the center of the screen
		CF_M3x2 projection = ortho_2d(0, 0, 640, 480);
		draw_projection(projection);

		// Set antialiasing
		draw_push_antialias(true);

		// Draw the spinning triangles with rainbow colors
		CF_SinCos sc = sincos(rotation * CF_PI / 180.0f);

		for (int triangle_idx = 0; triangle_idx < 3; triangle_idx++)
		{
			// Calculate rainbow color based on time and triangle index
			float time = CF_SECONDS;							// Use seconds since program start
			float hue = (time * 30.0f + triangle_idx * 120.0f); // Different hue for each triangle
			hue = fmod(hue, 360.0f);							// Keep hue in 0-360 range

			// Convert HSV to RGB for rainbow effect
			float h = hue / 60.0f;
			int i = (int)h;
			float f = h - i;
			float p = 0.0f;
			float q = 1.0f - f;
			float t = f;

			float r, g, b;
			switch (i % 6)
			{
			case 0:
				r = 1.0f;
				g = t;
				b = p;
				break;
			case 1:
				r = q;
				g = 1.0f;
				b = p;
				break;
			case 2:
				r = p;
				g = 1.0f;
				b = t;
				break;
			case 3:
				r = p;
				g = q;
				b = 1.0f;
				break;
			case 4:
				r = t;
				g = p;
				b = 1.0f;
				break;
			case 5:
				r = 1.0f;
				g = p;
				b = q;
				break;
			default:
				r = 1.0f;
				g = 1.0f;
				b = 1.0f;
				break;
			}

			// Create rainbow color with pulsing effect
			float pulse = 0.5f + 0.5f * sin(time * 2.0f); // Slow pulse
			CF_Color rainbow_color = make_color(r * pulse, g * pulse, b * pulse, 1.0f);

			// Set drawing color for this triangle
			draw_push_color(rainbow_color);

			// Transform vertices by rotation and position
			v2 rotated_vertices[3];
			for (int i = 0; i < 3; i++)
			{
				// First rotate the vertex
				v2 rotated = mul(sc, triangle_vertices[i]);
				// Then translate to the triangle's position
				rotated_vertices[i] = cf_add_v2(rotated, triangle_positions[triangle_idx]);
			}

			// Draw the triangle as three connected lines
			draw_line(rotated_vertices[0], rotated_vertices[1], 2.0f);
			draw_line(rotated_vertices[1], rotated_vertices[2], 2.0f);
			draw_line(rotated_vertices[2], rotated_vertices[0], 2.0f);

			draw_pop_color();
		}

		// Draw text below the triangles
		// Set text color to a nice pink color
		draw_push_color(make_color(1.0f, 0.4f, 0.7f, 1.0f)); // Pink color

		// Set font size for comic-style text
		push_font_size(24.0f);

		// Position text below the triangles (negative Y to go down)
		v2 text_position = v2(0, 100); // Below the triangles

		// Draw the text
		draw_text("my girlfriend is so pretty", text_position);

		// Restore font size
		pop_font_size();

		draw_pop_color();

		draw_pop_antialias();

		app_draw_onto_screen(true);
	}

	destroy_app();
	return 0;
}
