#include "CFNativeCamera.h"
#include <algorithm>
#include <cmath>
#include <random>

CFNativeCamera::CFNativeCamera()
    : m_position(cf_v2(0.0f, 0.0f)), m_zoom(1.0f), m_min_zoom(0.25f), m_max_zoom(4.0f), m_is_applied(false),
      m_viewport_width(0.0f), m_viewport_height(0.0f), m_use_window_size(true),
      m_shake_intensity(0.0f), m_shake_duration(0.0f), m_shake_decay(2.0f), m_shake_offset(cf_v2(0.0f, 0.0f)),
      m_target_ptr(nullptr), m_target_pos(cf_v2(0.0f, 0.0f)), m_has_static_target(false),
      m_follow_speed(5.0f), m_follow_deadzone(cf_v2(0.0f, 0.0f)),
      m_is_moving_smooth(false), m_move_start(cf_v2(0.0f, 0.0f)), m_move_target(cf_v2(0.0f, 0.0f)),
      m_move_duration(0.0f), m_move_elapsed(0.0f),
      m_is_zooming(false), m_zoom_start(1.0f), m_zoom_target(1.0f),
      m_zoom_duration(0.0f), m_zoom_elapsed(0.0f)
{
    // By default, get viewport from window
    updateViewportFromWindow();
}

CFNativeCamera::CFNativeCamera(v2 position, float zoom)
    : CFNativeCamera()
{ // Delegate to default constructor
    m_position = position;
    m_zoom = std::clamp(zoom, m_min_zoom, m_max_zoom);
}

CFNativeCamera::CFNativeCamera(v2 position, float zoom, float viewport_width, float viewport_height)
    : CFNativeCamera()
{ // Delegate to default constructor
    m_position = position;
    m_zoom = std::clamp(zoom, m_min_zoom, m_max_zoom);
    setViewportSize(viewport_width, viewport_height);
}

void CFNativeCamera::apply()
{
    if (m_is_applied)
        return;

    // Use CF's built-in transform system
    cf_draw_push();

    // Camera transform: First scale, then translate by scaled amount
    // This ensures the camera position stays at the center regardless of zoom
    v2 final_position = cf_v2(m_position.x + m_shake_offset.x, m_position.y + m_shake_offset.y);
    cf_draw_scale(m_zoom, m_zoom);
    cf_draw_translate(-final_position.x, -final_position.y);

    m_is_applied = true;
}

void CFNativeCamera::restore()
{
    if (!m_is_applied)
        return;

    // Restore CF's transform state
    cf_draw_pop();

    m_is_applied = false;
}

void CFNativeCamera::reset()
{
    m_position = cf_v2(0.0f, 0.0f);
    m_zoom = 1.0f;
}

void CFNativeCamera::setPosition(v2 position)
{
    m_position = position;
}

void CFNativeCamera::setPosition(float x, float y)
{
    m_position = cf_v2(x, y);
}

v2 CFNativeCamera::getPosition() const
{
    return m_position;
}

void CFNativeCamera::translate(v2 offset)
{
    m_position.x += offset.x;
    m_position.y += offset.y;
}

void CFNativeCamera::translate(float dx, float dy)
{
    m_position.x += dx;
    m_position.y += dy;
}

void CFNativeCamera::setZoom(float zoom)
{
    m_zoom = std::clamp(zoom, m_min_zoom, m_max_zoom);
}

float CFNativeCamera::getZoom() const
{
    return m_zoom;
}

void CFNativeCamera::zoomIn(float factor)
{
    setZoom(m_zoom * factor);
}

void CFNativeCamera::zoomOut(float factor)
{
    setZoom(m_zoom / factor);
}

void CFNativeCamera::setZoomRange(float min_zoom, float max_zoom)
{
    m_min_zoom = std::max(0.01f, min_zoom);
    m_max_zoom = std::max(m_min_zoom, max_zoom);
    m_zoom = std::clamp(m_zoom, m_min_zoom, m_max_zoom);
}

void CFNativeCamera::setViewportSize(float width, float height)
{
    m_viewport_width = width;
    m_viewport_height = height;
    m_use_window_size = false; // We're now using explicit viewport size
}

void CFNativeCamera::setViewportSize(v2 size)
{
    setViewportSize(size.x, size.y);
}

v2 CFNativeCamera::getViewportSize() const
{
    if (m_use_window_size)
    {
        return cf_v2((float)cf_app_get_width(), (float)cf_app_get_height());
    }
    return cf_v2(m_viewport_width, m_viewport_height);
}

void CFNativeCamera::updateViewportFromWindow()
{
    m_viewport_width = (float)cf_app_get_width();
    m_viewport_height = (float)cf_app_get_height();
    m_use_window_size = true; // Re-enable window size tracking
}

void CFNativeCamera::handleInput(float dt)
{
    float camera_speed = 200.0f; // pixels per second

    // WASD movement
    if (cf_key_down(CF_KEY_W) || cf_key_down(CF_KEY_UP))
    {
        translate(0.0f, camera_speed * dt);
    }
    if (cf_key_down(CF_KEY_S) || cf_key_down(CF_KEY_DOWN))
    {
        translate(0.0f, -camera_speed * dt);
    }
    if (cf_key_down(CF_KEY_A) || cf_key_down(CF_KEY_LEFT))
    {
        translate(-camera_speed * dt, 0.0f);
    }
    if (cf_key_down(CF_KEY_D) || cf_key_down(CF_KEY_RIGHT))
    {
        translate(camera_speed * dt, 0.0f);
    }

    // Q/E zoom
    if (cf_key_just_pressed(CF_KEY_Q))
    {
        zoomOut(1.2f);
    }
    if (cf_key_just_pressed(CF_KEY_E))
    {
        zoomIn(1.2f);
    }

    // R reset
    if (cf_key_just_pressed(CF_KEY_R))
    {
        reset();
    }
}

// Update camera (call once per frame)
void CFNativeCamera::update(float dt)
{
    updateShake(dt);
    updateFollowing(dt);
    updateSmoothMovement(dt);
}

// Camera shake methods
void CFNativeCamera::shake(float intensity, float duration)
{
    m_shake_intensity = intensity;
    m_shake_duration = duration;
}

void CFNativeCamera::setShakeDecay(float decay_rate)
{
    m_shake_decay = decay_rate;
}

void CFNativeCamera::stopShake()
{
    m_shake_intensity = 0.0f;
    m_shake_duration = 0.0f;
    m_shake_offset = cf_v2(0.0f, 0.0f);
}

// Target following methods
void CFNativeCamera::setTarget(v2 *target)
{
    m_target_ptr = target;
    m_has_static_target = false;
}

void CFNativeCamera::setTarget(v2 target)
{
    m_target_pos = target;
    m_target_ptr = nullptr;
    m_has_static_target = true;
}

void CFNativeCamera::clearTarget()
{
    m_target_ptr = nullptr;
    m_has_static_target = false;
}

void CFNativeCamera::setFollowSpeed(float speed)
{
    m_follow_speed = speed;
}

void CFNativeCamera::setFollowDeadzone(v2 deadzone)
{
    m_follow_deadzone = deadzone;
}

// Smooth movement methods
void CFNativeCamera::moveTo(v2 target_position, float duration)
{
    if (duration <= 0.0f)
    {
        m_position = target_position;
        return;
    }

    m_is_moving_smooth = true;
    m_move_start = m_position;
    m_move_target = target_position;
    m_move_duration = duration;
    m_move_elapsed = 0.0f;
}

void CFNativeCamera::zoomTo(float target_zoom, float duration)
{
    target_zoom = std::clamp(target_zoom, m_min_zoom, m_max_zoom);

    if (duration <= 0.0f)
    {
        m_zoom = target_zoom;
        return;
    }

    m_is_zooming = true;
    m_zoom_start = m_zoom;
    m_zoom_target = target_zoom;
    m_zoom_duration = duration;
    m_zoom_elapsed = 0.0f;
}

bool CFNativeCamera::isMoving() const
{
    return m_is_moving_smooth || m_is_zooming;
}

void CFNativeCamera::stopMovement()
{
    m_is_moving_smooth = false;
    m_is_zooming = false;
}

// Private helper methods
void CFNativeCamera::updateShake(float dt)
{
    if (m_shake_duration <= 0.0f)
    {
        m_shake_offset = cf_v2(0.0f, 0.0f);
        return;
    }

    // Decrease shake duration
    m_shake_duration -= dt;

    if (m_shake_duration <= 0.0f)
    {
        m_shake_offset = cf_v2(0.0f, 0.0f);
        return;
    }

    // Calculate shake offset with random direction
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<float> dis(-1.0f, 1.0f);

    float current_intensity = m_shake_intensity * (m_shake_duration > 0 ? std::pow(m_shake_duration, 1.0f / m_shake_decay) : 0.0f);
    m_shake_offset.x = dis(gen) * current_intensity;
    m_shake_offset.y = dis(gen) * current_intensity;
}

void CFNativeCamera::updateFollowing(float dt)
{
    v2 target;
    bool has_target = false;

    if (m_target_ptr && m_target_ptr != nullptr)
    {
        target = *m_target_ptr;
        has_target = true;
    }
    else if (m_has_static_target)
    {
        target = m_target_pos;
        has_target = true;
    }

    if (!has_target)
        return;

    // Calculate distance to target
    v2 diff = cf_v2(target.x - m_position.x, target.y - m_position.y);
    float dist_x = std::abs(diff.x);
    float dist_y = std::abs(diff.y);

    // Only move if outside deadzone
    if (dist_x > m_follow_deadzone.x)
    {
        float move_amount_x = (dist_x - m_follow_deadzone.x) * (diff.x > 0 ? 1.0f : -1.0f);
        m_position.x = lerp(m_position.x, m_position.x + move_amount_x, m_follow_speed * dt);
    }

    if (dist_y > m_follow_deadzone.y)
    {
        float move_amount_y = (dist_y - m_follow_deadzone.y) * (diff.y > 0 ? 1.0f : -1.0f);
        m_position.y = lerp(m_position.y, m_position.y + move_amount_y, m_follow_speed * dt);
    }
}

void CFNativeCamera::updateSmoothMovement(float dt)
{
    // Update smooth position movement
    if (m_is_moving_smooth)
    {
        m_move_elapsed += dt;
        float t = std::clamp(m_move_elapsed / m_move_duration, 0.0f, 1.0f);

        m_position = lerp(m_move_start, m_move_target, t);

        if (t >= 1.0f)
        {
            m_is_moving_smooth = false;
        }
    }

    // Update smooth zoom movement
    if (m_is_zooming)
    {
        m_zoom_elapsed += dt;
        float t = std::clamp(m_zoom_elapsed / m_zoom_duration, 0.0f, 1.0f);

        m_zoom = lerp(m_zoom_start, m_zoom_target, t);

        if (t >= 1.0f)
        {
            m_is_zooming = false;
        }
    }
}

float CFNativeCamera::lerp(float a, float b, float t) const
{
    return a + t * (b - a);
}

v2 CFNativeCamera::lerp(v2 a, v2 b, float t) const
{
    return cf_v2(lerp(a.x, b.x, t), lerp(a.y, b.y, t));
}

CF_Aabb CFNativeCamera::getViewBounds() const
{
    // Use viewport dimensions (which may be scaled from window size)
    // This ensures view bounds match what's actually being rendered
    v2 viewport_size = getViewportSize();
    float screen_width = viewport_size.x;
    float screen_height = viewport_size.y;

    // Calculate world-space dimensions based on zoom
    // At zoom=1.0, screen size = world size
    // At zoom=2.0, we see half the world space (zoomed in)
    float world_width = screen_width / m_zoom;
    float world_height = screen_height / m_zoom;

    // Calculate camera position including shake offset
    v2 effective_position = cf_v2(m_position.x + m_shake_offset.x,
                                  m_position.y + m_shake_offset.y);

    // Camera position is the center, so calculate min/max bounds
    float half_width = world_width * 0.5f;
    float half_height = world_height * 0.5f;

    CF_Aabb result = make_aabb(
        cf_v2(effective_position.x - half_width, effective_position.y - half_height),
        cf_v2(effective_position.x + half_width, effective_position.y + half_height));

    static int frame_count = 0;
    if (false && frame_count++ % 60 == 0)
    {
        printf("ViewBounds: screen=%.0fx%.0f, zoom=%.2f, world=%.0fx%.0f, pos=(%.0f,%.0f), bounds=(%.0f,%.0f)-(%.0f,%.0f)\n",
               screen_width, screen_height, m_zoom, world_width, world_height,
               effective_position.x, effective_position.y,
               result.min.x, result.min.y, result.max.x, result.max.y);
    }

    return result;
}

bool CFNativeCamera::isVisible(CF_Aabb bounds) const
{
    CF_Aabb view_bounds = getViewBounds();

    // Check for AABB intersection
    return !(bounds.max.x < view_bounds.min.x ||
             bounds.min.x > view_bounds.max.x ||
             bounds.max.y < view_bounds.min.y ||
             bounds.min.y > view_bounds.max.y);
}

void CFNativeCamera::drawDebugInfo(float x, float y) const
{
    // Enhanced debug text showing camera state including new features
    char info[512];
    snprintf(info, sizeof(info),
             "CF Camera: pos=(%.1f, %.1f) zoom=%.2f shake=%.1f target=%s smooth=%s",
             m_position.x, m_position.y, m_zoom, m_shake_intensity,
             (m_target_ptr || m_has_static_target) ? "YES" : "NO",
             isMoving() ? "YES" : "NO");

    v2 text_pos = cf_v2(x, y);
    draw_text(info, text_pos);
}
