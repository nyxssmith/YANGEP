#include "Camera.h"
#include <cute.h>
#include <cmath>
#include <algorithm>
#include "DebugPrint.h"

// Constants
static const float DEFAULT_MIN_ZOOM = 0.1f;
static const float DEFAULT_MAX_ZOOM = 10.0f;
static const float DEFAULT_FOLLOW_SPEED = 5.0f;
static const float DEFAULT_SHAKE_DECAY = 2.0f;

// Default constructor
Camera::Camera()
    : m_position(cf_v2(0.0f, 0.0f)), m_zoom(1.0f), m_rotation(0.0f), m_viewport_size(cf_v2(640.0f, 480.0f)), m_auto_viewport(true), m_min_zoom(DEFAULT_MIN_ZOOM), m_max_zoom(DEFAULT_MAX_ZOOM), m_has_world_bounds(false), m_world_min(cf_v2(0.0f, 0.0f)), m_world_max(cf_v2(0.0f, 0.0f)), m_target_ptr(nullptr), m_target_pos(cf_v2(0.0f, 0.0f)), m_has_static_target(false), m_follow_speed(DEFAULT_FOLLOW_SPEED), m_follow_deadzone(cf_v2(0.0f, 0.0f)), m_follow_offset(cf_v2(0.0f, 0.0f)), m_shake_intensity(0.0f), m_shake_duration(0.0f), m_shake_decay(DEFAULT_SHAKE_DECAY), m_shake_offset(cf_v2(0.0f, 0.0f)), m_is_moving(false), m_move_start(cf_v2(0.0f, 0.0f)), m_move_target(cf_v2(0.0f, 0.0f)), m_move_duration(0.0f), m_move_elapsed(0.0f), m_is_zooming(false), m_zoom_start(1.0f), m_zoom_target(1.0f), m_zoom_duration(0.0f), m_zoom_elapsed(0.0f), m_is_rotating(false), m_rotation_start(0.0f), m_rotation_target(0.0f), m_rotation_duration(0.0f), m_rotation_elapsed(0.0f), m_is_applied(false), m_matrices_dirty(true)
{
    updateViewportFromWindow();
    updateMatrices();
}

// Constructor with position and zoom
Camera::Camera(v2 position, float zoom)
    : Camera() // Delegate to default constructor
{
    m_position = position;
    m_zoom = std::clamp(zoom, m_min_zoom, m_max_zoom);
    m_matrices_dirty = true;
}

// Destructor
Camera::~Camera()
{
    if (m_is_applied)
    {
        restore();
    }
}

// Update camera (call once per frame)
void Camera::update(float dt)
{
    bool position_changed = false;

    // Update auto viewport if enabled
    if (m_auto_viewport)
    {
        updateViewportFromWindow();
    }

    // Update shake effect
    updateShake(dt);

    // Update target following
    updateTargetFollowing(dt);
    position_changed = true; // Target following can change position

    // Update smooth movements
    updateSmoothMovement(dt);

    // Apply world bounds if set
    if (m_has_world_bounds)
    {
        v2 old_pos = m_position;
        applyWorldBounds();
        if (old_pos.x != m_position.x || old_pos.y != m_position.y)
        {
            position_changed = true;
        }
    }

    // Update matrices if needed
    if (m_matrices_dirty || position_changed)
    {
        updateMatrices();
    }
}

void Camera::drawDebugGrid()
{

    // Draw debug objects in world space
    // Main reference square at origin
    draw_quad(CF_Aabb{cf_v2(-50.0f, -50.0f), cf_v2(50.0f, 50.0f)}, 2.0f, 0.0f);

    // Grid of smaller squares to show camera movement
    for (int x = -5; x <= 5; x++)
    {
        for (int y = -5; y <= 5; y++)
        {
            if (x == 0 && y == 0)
                continue; // Skip the center square

            float square_x = x * 150.0f;
            float square_y = y * 150.0f;
            CF_Color color = cf_color_blue();

            // Make some squares different colors for reference
            if (x == 0 || y == 0)
                color = cf_color_green(); // Axis squares
            if (std::abs(x) == 1 && std::abs(y) == 1)
                color = cf_color_yellow(); // Corner squares

            draw_quad(CF_Aabb{cf_v2(square_x - 25.0f, square_y - 25.0f),
                              cf_v2(square_x + 25.0f, square_y + 25.0f)},
                      2.0f, 0.0f);
        }
    }

    // Draw coordinate axes
    // X-axis (horizontal red line)
    draw_line(cf_v2(-4000.0f, 0.0f), cf_v2(4000.0f, 0.0f), 3.0f);
    // Y-axis (vertical green line)
    draw_line(cf_v2(0.0f, -4000.0f), cf_v2(0.0f, 4000.0f), 3.0f);
}

// Apply camera transformation to Cute Framework
void Camera::apply()
{
    if (m_is_applied)
    {
        DebugPrint::Print("Camera", "Warning: Camera::apply() called when camera is already applied. Call restore() first.\n");
        return;
    }

    // Push current transform state
    cf_draw_push();

    // Apply the view transformation
    // The cute framework uses standard 2D graphics transforms
    // We need to apply in reverse order: translate -> rotate -> scale

    // Get the final camera position including shake
    v2 final_position = m_position + m_shake_offset;

    // Aggressive pixel alignment to prevent seams at any zoom level
    // We need to ensure that after zoom transformation, everything aligns to pixel boundaries
    if (m_zoom >= 1.0f)
    {
        // For zoom >= 1, round to 1/zoom precision to ensure pixel alignment after scaling
        float zoom_precision = 1.0f / m_zoom;
        final_position.x = floorf(final_position.x / zoom_precision) * zoom_precision;
        final_position.y = floorf(final_position.y / zoom_precision) * zoom_precision;
    }
    else
    {
        // For zoom < 1, just round to integer pixels
        final_position.x = roundf(final_position.x);
        final_position.y = roundf(final_position.y);
    }

    // Apply zoom (scale)
    cf_draw_scale(m_zoom, m_zoom);

    // Apply rotation around camera center
    if (m_rotation != 0.0f)
    {
        cf_draw_rotate(m_rotation);
    }

    // Apply translation (move world opposite to camera movement)
    cf_draw_translate(-final_position.x, -final_position.y);
    m_is_applied = true;
}

// Restore previous transformation state
void Camera::restore()
{
    if (!m_is_applied)
    {
        DebugPrint::Print("Camera", "Warning: Camera::restore() called when camera is not applied.\n");
        return;
    }

    cf_draw_pop();
    m_is_applied = false;
}

// Position control
void Camera::setPosition(v2 position)
{
    m_position = position;
    m_matrices_dirty = true;
}

void Camera::setPosition(float x, float y)
{
    setPosition(cf_v2(x, y));
}

v2 Camera::getPosition() const
{
    return m_position;
}

void Camera::translate(v2 offset)
{
    m_position = m_position + offset;
    m_matrices_dirty = true;
}

void Camera::translate(float dx, float dy)
{
    translate(cf_v2(dx, dy));
}

// Zoom control
void Camera::setZoom(float zoom)
{
    m_zoom = std::clamp(zoom, m_min_zoom, m_max_zoom);
    m_matrices_dirty = true;
}

float Camera::getZoom() const
{
    return m_zoom;
}

void Camera::zoomIn(float factor)
{
    setZoom(m_zoom * factor);
}

void Camera::zoomOut(float factor)
{
    setZoom(m_zoom / factor);
}

void Camera::setZoomRange(float min_zoom, float max_zoom)
{
    m_min_zoom = min_zoom;
    m_max_zoom = max_zoom;
    m_zoom = std::clamp(m_zoom, m_min_zoom, m_max_zoom);
    m_matrices_dirty = true;
}

// Rotation control
void Camera::setRotation(float radians)
{
    m_rotation = radians;
    m_matrices_dirty = true;
}

float Camera::getRotation() const
{
    return m_rotation;
}

void Camera::rotate(float radians)
{
    m_rotation += radians;
    m_matrices_dirty = true;
}

// Viewport control
void Camera::setViewport(float width, float height)
{
    m_viewport_size = cf_v2(width, height);
    m_auto_viewport = false;
    m_matrices_dirty = true;
}

void Camera::setViewport(v2 size)
{
    setViewport(size.x, size.y);
}

v2 Camera::getViewportSize() const
{
    return m_viewport_size;
}

void Camera::updateViewportFromWindow()
{
    int width = cf_app_get_width();
    int height = cf_app_get_height();

    if (width != (int)m_viewport_size.x || height != (int)m_viewport_size.y)
    {
        m_viewport_size = cf_v2((float)width, (float)height);
        m_matrices_dirty = true;
    }
}

// World bounds
void Camera::setWorldBounds(v2 min, v2 max)
{
    m_has_world_bounds = true;
    m_world_min = min;
    m_world_max = max;
}

void Camera::setWorldBounds(float min_x, float min_y, float max_x, float max_y)
{
    setWorldBounds(cf_v2(min_x, min_y), cf_v2(max_x, max_y));
}

void Camera::clearWorldBounds()
{
    m_has_world_bounds = false;
}

bool Camera::hasWorldBounds() const
{
    return m_has_world_bounds;
}

// Coordinate conversions
v2 Camera::screenToWorld(v2 screen_pos) const
{
    // Convert from screen space to world space
    // Screen space: (0,0) at top-left, +X right, +Y down
    // World space: camera-relative coordinates

    // Center the screen coordinates
    v2 centered = screen_pos - (m_viewport_size * 0.5f);

    // Apply inverse zoom
    centered = centered / m_zoom;

    // Apply inverse rotation
    if (m_rotation != 0.0f)
    {
        float cos_r = CF_COSF(-m_rotation);
        float sin_r = CF_SINF(-m_rotation);
        v2 rotated;
        rotated.x = centered.x * cos_r - centered.y * sin_r;
        rotated.y = centered.x * sin_r + centered.y * cos_r;
        centered = rotated;
    }

    // Add camera position (with shake)
    return centered + m_position + m_shake_offset;
}

v2 Camera::worldToScreen(v2 world_pos) const
{
    // Convert from world space to screen space

    // Subtract camera position (with shake)
    v2 relative = world_pos - (m_position + m_shake_offset);

    // Apply rotation
    if (m_rotation != 0.0f)
    {
        float cos_r = CF_COSF(m_rotation);
        float sin_r = CF_SINF(m_rotation);
        v2 rotated;
        rotated.x = relative.x * cos_r - relative.y * sin_r;
        rotated.y = relative.x * sin_r + relative.y * cos_r;
        relative = rotated;
    }

    // Apply zoom
    relative = relative * m_zoom;

    // Convert to screen coordinates (add viewport center)
    return relative + (m_viewport_size * 0.5f);
}

v2 Camera::screenToWorld(float screen_x, float screen_y) const
{
    return screenToWorld(cf_v2(screen_x, screen_y));
}

v2 Camera::worldToScreen(float world_x, float world_y) const
{
    return worldToScreen(cf_v2(world_x, world_y));
}

// Camera bounds and visibility
CF_Aabb Camera::getViewBounds() const
{
    // Calculate world-space bounds of what the camera can see
    v2 half_viewport = m_viewport_size * 0.5f / m_zoom;
    v2 center = m_position + m_shake_offset;

    // For simplicity, ignore rotation in bounds calculation
    // Real implementation might want to handle rotation properly
    return make_aabb(center - half_viewport, center + half_viewport);
}

CF_Aabb Camera::getScreenBounds() const
{
    return make_aabb(cf_v2(0, 0), m_viewport_size);
}

bool Camera::isVisible(v2 world_pos, float radius) const
{
    CF_Aabb view_bounds = getViewBounds();

    if (radius <= 0.0f)
    {
        // Point visibility
        return contains(view_bounds, world_pos);
    }
    else
    {
        // Circle visibility
        CF_Circle circle = make_circle(world_pos, radius);
        return cf_circle_to_aabb(circle, view_bounds);
    }
}

bool Camera::isVisible(CF_Aabb world_bounds) const
{
    CF_Aabb view_bounds = getViewBounds();
    return overlaps(view_bounds, world_bounds);
}

// Target following
void Camera::setTarget(v2 *target)
{
    m_target_ptr = target;
    m_has_static_target = false;
}

void Camera::setTarget(v2 target)
{
    m_target_pos = target;
    m_target_ptr = nullptr;
    m_has_static_target = true;
}

void Camera::clearTarget()
{
    m_target_ptr = nullptr;
    m_has_static_target = false;
}

void Camera::setFollowSpeed(float speed)
{
    m_follow_speed = speed;
}

void Camera::setFollowDeadzone(v2 deadzone)
{
    m_follow_deadzone = deadzone;
}

void Camera::setFollowOffset(v2 offset)
{
    m_follow_offset = offset;
}

// Camera shake
void Camera::shake(float intensity, float duration)
{
    m_shake_intensity = intensity;
    m_shake_duration = duration;
}

void Camera::setShakeDecay(float decay_rate)
{
    m_shake_decay = decay_rate;
}

void Camera::stopShake()
{
    m_shake_intensity = 0.0f;
    m_shake_duration = 0.0f;
    m_shake_offset = cf_v2(0.0f, 0.0f);
}

// Smooth movement
void Camera::moveTo(v2 target_position, float duration)
{
    m_is_moving = true;
    m_move_start = m_position;
    m_move_target = target_position;
    m_move_duration = duration;
    m_move_elapsed = 0.0f;
}

void Camera::zoomTo(float target_zoom, float duration)
{
    m_is_zooming = true;
    m_zoom_start = m_zoom;
    m_zoom_target = std::clamp(target_zoom, m_min_zoom, m_max_zoom);
    m_zoom_duration = duration;
    m_zoom_elapsed = 0.0f;
}

void Camera::rotateTo(float target_rotation, float duration)
{
    m_is_rotating = true;
    m_rotation_start = m_rotation;
    m_rotation_target = target_rotation;
    m_rotation_duration = duration;
    m_rotation_elapsed = 0.0f;
}

bool Camera::isMoving() const
{
    return m_is_moving || m_is_zooming || m_is_rotating;
}

void Camera::stopMovement()
{
    m_is_moving = false;
    m_is_zooming = false;
    m_is_rotating = false;
}

// Matrix operations
CF_M3x2 Camera::getViewMatrix() const
{
    return m_view_matrix;
}

CF_M3x2 Camera::getProjectionMatrix() const
{
    return m_projection_matrix;
}

CF_M3x2 Camera::getViewProjectionMatrix() const
{
    return m_view_projection_matrix;
}

// Utility functions
void Camera::reset()
{
    m_position = cf_v2(0.0f, 0.0f);
    m_zoom = 1.0f;
    m_rotation = 0.0f;
    stopShake();
    stopMovement();
    clearTarget();
    m_matrices_dirty = true;
}

void Camera::centerOnPoint(v2 point)
{
    m_position = point;
    m_matrices_dirty = true;
}

void Camera::fitToView(CF_Aabb world_bounds, float padding)
{
    // Calculate the zoom needed to fit the bounds in the viewport
    v2 bounds_size = extents(world_bounds);
    v2 viewport_size_world = m_viewport_size;

    // Add padding
    bounds_size = bounds_size * (1.0f + padding);

    // Calculate zoom to fit both width and height
    float zoom_x = viewport_size_world.x / bounds_size.x;
    float zoom_y = viewport_size_world.y / bounds_size.y;
    float new_zoom = std::min(zoom_x, zoom_y);

    // Center on the bounds
    v2 bounds_center = center(world_bounds);

    setZoom(new_zoom);
    centerOnPoint(bounds_center);
}

// Debug helpers
void Camera::drawDebugInfo(float x, float y) const
{
    // Draw camera info text at specified (x, y)
    char info[256];
    snprintf(info, sizeof(info), "Camera: (%.1f, %.1f) Zoom: %.2f Rot: %.2f°",
             m_position.x, m_position.y, m_zoom, m_rotation * 180.0f / CF_PI);

    draw_text(info, cf_v2(x, y));
    /*
    // Draw target info if following
    if (m_target_ptr || m_has_static_target)
    {
        v2 target = getCurrentTarget();
        snprintf(info, sizeof(info), "Target: (%.1f, %.1f)", target.x, target.y);
        draw_text(info, cf_v2(x, y + 20.0f));
    }

    // Draw shake info if shaking
    if (m_shake_intensity > 0.0f)
    {
        snprintf(info, sizeof(info), "Shake: %.2f (%.2fs)", m_shake_intensity, m_shake_duration);
        draw_text(info, cf_v2(x, y + 40.0f));
    }
    */
}

void Camera::drawViewBounds() const
{
    CF_Aabb bounds = getViewBounds();
    draw_quad(bounds, 2.0f);
}

// Private helper functions
void Camera::updateShake(float dt)
{
    if (m_shake_duration > 0.0f)
    {
        m_shake_duration -= dt;

        if (m_shake_duration <= 0.0f)
        {
            // Shake finished
            m_shake_intensity = 0.0f;
            m_shake_duration = 0.0f;
            m_shake_offset = cf_v2(0.0f, 0.0f);
        }
        else
        {
            // Generate shake offset
            float shake_amount = m_shake_intensity * (m_shake_duration / m_shake_decay);

            // Create a simple random-like shake using time
            // Not perfect randomness but good enough for shake effect
            static float shake_time = 0.0f;
            shake_time += dt * 50.0f; // Fast oscillation

            float angle1 = shake_time * 2.7f; // Prime-ish numbers for irregular pattern
            float angle2 = shake_time * 3.1f;

            m_shake_offset.x = CF_COSF(angle1) * shake_amount * 0.5f;
            m_shake_offset.y = CF_SINF(angle2) * shake_amount * 0.5f;
        }

        m_matrices_dirty = true;
    }
}

void Camera::updateTargetFollowing(float dt)
{
    if (m_target_ptr || m_has_static_target)
    {
        v2 target = getCurrentTarget();
        v2 target_world = target + m_follow_offset;

        // Check if target is outside deadzone
        v2 diff = target_world - m_position;
        v2 abs_diff = abs(diff);

        bool outside_deadzone = (abs_diff.x > m_follow_deadzone.x || abs_diff.y > m_follow_deadzone.y);

        if (outside_deadzone)
        {
            // Move toward target
            if (m_follow_speed <= 0.0f)
            {
                // Instant follow
                m_position = target_world;
            }
            else
            {
                // Smooth follow
                float lerp_factor = std::min(1.0f, m_follow_speed * dt);
                m_position = lerp(m_position, target_world, lerp_factor);
            }
            m_matrices_dirty = true;
        }
    }
}

void Camera::updateSmoothMovement(float dt)
{
    bool any_movement = false;

    // Update position movement
    if (m_is_moving)
    {
        m_move_elapsed += dt;
        float t = std::min(1.0f, m_move_elapsed / m_move_duration);

        // Use smoothstep for easing
        t = smoothstep(t);

        m_position = lerp(m_move_start, m_move_target, t);

        if (t >= 1.0f)
        {
            m_is_moving = false;
        }
        any_movement = true;
    }

    // Update zoom movement
    if (m_is_zooming)
    {
        m_zoom_elapsed += dt;
        float t = std::min(1.0f, m_zoom_elapsed / m_zoom_duration);

        t = smoothstep(t);
        m_zoom = cf_lerp(m_zoom_start, m_zoom_target, t);

        if (t >= 1.0f)
        {
            m_is_zooming = false;
        }
        any_movement = true;
    }

    // Update rotation movement
    if (m_is_rotating)
    {
        m_rotation_elapsed += dt;
        float t = std::min(1.0f, m_rotation_elapsed / m_rotation_duration);

        t = smoothstep(t);
        m_rotation = lerpAngle(m_rotation_start, m_rotation_target, t);

        if (t >= 1.0f)
        {
            m_is_rotating = false;
        }
        any_movement = true;
    }

    if (any_movement)
    {
        m_matrices_dirty = true;
    }
}

void Camera::applyWorldBounds()
{
    if (!m_has_world_bounds)
        return;

    // Keep camera within world bounds, accounting for viewport size and zoom
    v2 half_viewport = m_viewport_size * 0.5f / m_zoom;

    // Clamp camera position so the viewport stays within world bounds
    m_position.x = std::clamp(m_position.x, m_world_min.x + half_viewport.x, m_world_max.x - half_viewport.x);
    m_position.y = std::clamp(m_position.y, m_world_min.y + half_viewport.y, m_world_max.y - half_viewport.y);
}

v2 Camera::getCurrentTarget() const
{
    if (m_target_ptr)
    {
        return *m_target_ptr;
    }
    else if (m_has_static_target)
    {
        return m_target_pos;
    }
    return m_position; // Fallback
}

float Camera::lerpAngle(float start, float end, float t) const
{
    // Handle angle wrapping for smooth rotation
    float diff = end - start;

    // Normalize to [-PI, PI]
    while (diff > CF_PI)
        diff -= 2.0f * CF_PI;
    while (diff < -CF_PI)
        diff += 2.0f * CF_PI;

    return start + diff * t;
}

void Camera::updateMatrices()
{
    // Create view matrix (world to camera space)
    v2 final_position = m_position + m_shake_offset;

    // View matrix: translate then rotate then scale
    CF_M3x2 translate_matrix = make_translation(-final_position);
    CF_M3x2 rotate_matrix = make_rotation(m_rotation);
    CF_M3x2 scale_matrix = make_scale(m_zoom);

    // Combine transformations: scale * rotate * translate
    m_view_matrix = mul(scale_matrix, mul(rotate_matrix, translate_matrix));

    // Create orthogonal projection matrix (camera to screen space)
    // cf_ortho_2d parameters: x, y, scale_x, scale_y
    // This creates a projection where (0,0) is at center of screen
    m_projection_matrix = cf_ortho_2d(0.0f, 0.0f, m_viewport_size.x, m_viewport_size.y);

    // Combine view and projection
    m_view_projection_matrix = mul(m_projection_matrix, m_view_matrix);

    m_matrices_dirty = false;
}
