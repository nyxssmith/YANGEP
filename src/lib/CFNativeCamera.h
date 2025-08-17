#pragma once

#include <cute.h>

using namespace Cute;

/**
 * @class CFNativeCamera
 * @brief Simple camera using CF's built-in transform system
 *
 * Uses cf_draw_push/pop, cf_draw_translate, cf_draw_scale instead of custom matrices
 */
class CFNativeCamera {
public:
    CFNativeCamera();
    CFNativeCamera(v2 position, float zoom = 1.0f);

    // Basic camera operations
    void apply();
    void restore();
    void reset();

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

    // Input handling
    void handleInput(float dt);

    // Camera shake
    void shake(float intensity, float duration);
    void setShakeDecay(float decay_rate = 2.0f);
    void stopShake();

    // Target following
    void setTarget(v2* target);
    void setTarget(v2 target);
    void clearTarget();
    void setFollowSpeed(float speed);
    void setFollowDeadzone(v2 deadzone);

    // Smooth movement
    void moveTo(v2 target_position, float duration);
    void zoomTo(float target_zoom, float duration);
    bool isMoving() const;
    void stopMovement();

    // Update (call once per frame)
    void update(float dt);

    // View bounds and visibility
    CF_Aabb getViewBounds() const;
    bool isVisible(CF_Aabb bounds) const;

    // Debug
    void drawDebugInfo(float x, float y) const;

private:
    // Core state
    v2 m_position;
    float m_zoom;
    float m_min_zoom;
    float m_max_zoom;
    bool m_is_applied;

    // Camera shake
    float m_shake_intensity;
    float m_shake_duration;
    float m_shake_decay;
    v2 m_shake_offset;

    // Target following
    v2* m_target_ptr;
    v2 m_target_pos;
    bool m_has_static_target;
    float m_follow_speed;
    v2 m_follow_deadzone;

    // Smooth movement
    bool m_is_moving_smooth;
    v2 m_move_start;
    v2 m_move_target;
    float m_move_duration;
    float m_move_elapsed;

    bool m_is_zooming;
    float m_zoom_start;
    float m_zoom_target;
    float m_zoom_duration;
    float m_zoom_elapsed;

    // Helper functions
    void updateShake(float dt);
    void updateFollowing(float dt);
    void updateSmoothMovement(float dt);
    float lerp(float a, float b, float t) const;
    v2 lerp(v2 a, v2 b, float t) const;
};
