#pragma once

#include <cute.h>

using namespace Cute;

/**
 * @class Camera
 * @brief A 2D camera system for the Cute Framework that handles view transformations and orthogonal projection.
 *
 * The Camera class provides a comprehensive interface for:
 * - Managing 2D camera position, zoom, and rotation
 * - Setting up orthogonal projection matrices for 2D rendering
 * - Converting between world and screen coordinates
 * - Handling viewport and screen bounds
 * - Following targets with smooth camera movement
 * - Managing camera shake effects
 *
 * Coordinate System:
 * - World space: arbitrary units, camera position determines what's visible
 * - Screen space: pixels, (0,0) at top-left, +X right, +Y down
 * - Camera space: relative to camera position and zoom
 */
class Camera
{
public:
    // Constructor/Destructor
    Camera();
    Camera(v2 position, float zoom = 1.0f);
    ~Camera();

    // Core camera operations
    void update(float dt);
    void apply();
    void restore();
    void drawDebugGrid();
    // Position control
    void setPosition(v2 position);
    void setPosition(float x, float y);
    v2 getPosition() const;
    void translate(v2 offset);
    void translate(float dx, float dy);

    // Zoom control
    void setZoom(float zoom);
    float getZoom() const;
    void zoomIn(float factor = 1.1f);
    void zoomOut(float factor = 1.1f);
    void setZoomRange(float min_zoom, float max_zoom);

    // Rotation control
    void setRotation(float radians);
    float getRotation() const;
    void rotate(float radians);

    // Viewport and bounds
    void setViewport(float width, float height);
    void setViewport(v2 size);
    v2 getViewportSize() const;
    void updateViewportFromWindow(); // Auto-updates from cf_app_get_width/height

    // World bounds (optional camera limits)
    void setWorldBounds(v2 min, v2 max);
    void setWorldBounds(float min_x, float min_y, float max_x, float max_y);
    void clearWorldBounds();
    bool hasWorldBounds() const;

    // Coordinate conversions
    v2 screenToWorld(v2 screen_pos) const;
    v2 worldToScreen(v2 world_pos) const;
    v2 screenToWorld(float screen_x, float screen_y) const;
    v2 worldToScreen(float world_x, float world_y) const;

    // Camera bounds and visibility
    CF_Aabb getViewBounds() const;                           // World-space bounds of what camera sees
    CF_Aabb getScreenBounds() const;                         // Screen-space bounds
    bool isVisible(v2 world_pos, float radius = 0.0f) const; // Point visibility
    bool isVisible(CF_Aabb world_bounds) const;              // AABB visibility

    // Target following
    void setTarget(v2 *target);
    void setTarget(v2 target);
    void clearTarget();
    void setFollowSpeed(float speed);    // How quickly to follow target (0 = instant, 1 = slow)
    void setFollowDeadzone(v2 deadzone); // Area around target where camera doesn't move
    void setFollowOffset(v2 offset);     // Offset from target position

    // Camera shake
    void shake(float intensity, float duration);
    void setShakeDecay(float decay_rate = 2.0f);
    void stopShake();

    // Smooth movement
    void moveTo(v2 target_position, float duration);
    void zoomTo(float target_zoom, float duration);
    void rotateTo(float target_rotation, float duration);
    bool isMoving() const;
    void stopMovement();

    // Matrix operations
    CF_M3x2 getViewMatrix() const;
    CF_M3x2 getProjectionMatrix() const;
    CF_M3x2 getViewProjectionMatrix() const;

    // Utility functions
    void reset();                                               // Reset to default position/zoom/rotation
    void centerOnPoint(v2 point);                               // Instantly center camera on point
    void fitToView(CF_Aabb world_bounds, float padding = 0.1f); // Adjust zoom to fit bounds

    // Debug helpers
    void drawDebugInfo() const;  // Draw camera bounds and info
    void drawViewBounds() const; // Draw world-space view bounds

private:
    // Core camera state
    v2 m_position;    // Camera position in world space
    float m_zoom;     // Zoom level (1.0 = normal, 2.0 = 2x magnification)
    float m_rotation; // Rotation in radians

    // Viewport
    v2 m_viewport_size;   // Viewport dimensions in pixels
    bool m_auto_viewport; // Auto-update viewport from window size

    // Zoom constraints
    float m_min_zoom;
    float m_max_zoom;

    // World bounds (optional)
    bool m_has_world_bounds;
    v2 m_world_min;
    v2 m_world_max;

    // Target following
    v2 *m_target_ptr;         // Pointer to target position (can be null)
    v2 m_target_pos;          // Static target position
    bool m_has_static_target; // Whether we're using static target
    float m_follow_speed;     // Follow interpolation speed
    v2 m_follow_deadzone;     // Deadzone around target
    v2 m_follow_offset;       // Offset from target

    // Camera shake
    float m_shake_intensity;
    float m_shake_duration;
    float m_shake_decay;
    v2 m_shake_offset;

    // Smooth movement
    bool m_is_moving;
    v2 m_move_start;
    v2 m_move_target;
    float m_move_duration;
    float m_move_elapsed;

    bool m_is_zooming;
    float m_zoom_start;
    float m_zoom_target;
    float m_zoom_duration;
    float m_zoom_elapsed;

    bool m_is_rotating;
    float m_rotation_start;
    float m_rotation_target;
    float m_rotation_duration;
    float m_rotation_elapsed;

    // Matrix stack management
    bool m_is_applied;

    // Internal helper functions
    void updateShake(float dt);
    void updateTargetFollowing(float dt);
    void updateSmoothMovement(float dt);
    void applyWorldBounds();
    v2 getCurrentTarget() const;
    float lerpAngle(float start, float end, float t) const;

    // Matrix calculations
    void updateMatrices();
    CF_M3x2 m_view_matrix;
    CF_M3x2 m_projection_matrix;
    CF_M3x2 m_view_projection_matrix;
    bool m_matrices_dirty;
};
