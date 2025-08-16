#include "AnimationDemo.h"
#include <cute.h>
#include <sstream>
#include <iomanip>

AnimationDemo::AnimationDemo()
    : demoTime(0.0f)
    , showIdle(true)
    , showWalkCycle(false)
    , currentDirection(SkeletonDirection::DOWN)
    , animationSpeed(1.0f)
    , animationPaused(false)
    , current_body(nullptr)
    , current_head(nullptr)
{
}

AnimationDemo::~AnimationDemo() {
    // Cleanup handled automatically
}

bool AnimationDemo::initialize() {
    printf("AnimationDemo::initialize() - Starting initialization...\n");

    // Create animated sprites for different states
    // Idle sprites (single frame)
    printf("Creating idle sprites...\n");
    skeleton_body_idle = AnimatedSprite("skeleton_body_idle", "assets/Art/AnimationsSheets/idle/BODY_skeleton.png");
    skeleton_head_idle = AnimatedSprite("skeleton_head_idle", "assets/Art/AnimationsSheets/idle/HEAD_chain_armor_helmet.png");

    // Walk cycle sprites (multi-frame)
    printf("Creating walk cycle sprites...\n");
    skeleton_body_walk = AnimatedSprite("skeleton_body_walk", "assets/Art/AnimationsSheets/walkcycle/BODY_skeleton.png");
    skeleton_head_walk = AnimatedSprite("skeleton_head_walk", "assets/Art/AnimationsSheets/walkcycle/HEAD_plate_armor_helmet.png");

        // Set up animations
    printf("Setting up animations...\n");
    setupAnimations();

    // Get screen dimensions and center sprites
    printf("Positioning sprites...\n");
    v2 screen_size(640, 480); // Default size, will be updated when window is resized
    v2 center = v2(0, 0); // With orthographic projection, (0,0) is at center of screen

    printf("Screen size: (%.1f, %.1f), Center: (%.1f, %.1f)\n",
           screen_size.x, screen_size.y, center.x, center.y);

    // Position all sprites at center of screen
    skeleton_body_idle.setPosition(center);
    skeleton_head_idle.setPosition(v2(center.x, center.y - 40));
    skeleton_body_walk.setPosition(center);
    skeleton_head_walk.setPosition(v2(center.x, center.y - 40));

    printf("Body sprite position: (%.1f, %.1f)\n",
           skeleton_body_idle.getPosition().x, skeleton_body_idle.getPosition().y);
    printf("Head sprite position: (%.1f, %.1f)\n",
           skeleton_head_idle.getPosition().x, skeleton_head_idle.getPosition().y);

    // Set initial active sprites
    printf("Setting initial animation state...\n");
    switchToAnimation(true); // Start with idle

    printf("AnimationDemo::initialize() - Initialization complete!\n");
    return true;
}

void AnimationDemo::setupAnimations() {
    // Set up idle animation (single frame for current direction)
    Animation idle_body("idle");
    idle_body.setFrameSize(v2(64, 256));
    idle_body.setSheetSize(v2(64, 256));
    idle_body.addFrame(0, static_cast<int>(currentDirection), 0.5f, "idle_frame");

    Animation idle_head("idle");
    idle_head.setFrameSize(v2(64, 256));
    idle_head.setSheetSize(v2(64, 256));
    idle_head.addFrame(0, static_cast<int>(currentDirection), 0.5f, "idle_frame");

    // Set up walk cycle animation for current direction
    Animation walk_body("walk");
    walk_body.setFrameSize(v2(64, 256));
    walk_body.setSheetSize(v2(576, 256)); // 9 frames × 4 directions
    walk_body.addWalkCycleForDirection(currentDirection, 0.1f);

    // Use walk cycle head asset (plate armor helmet) for current direction
    Animation walk_head("walk");
    walk_head.setFrameSize(v2(64, 256));
    walk_head.setSheetSize(v2(576, 256)); // 9 frames × 4 directions
    walk_head.addWalkCycleForDirection(currentDirection, 0.1f);

        // Add animations to sprites
    skeleton_body_idle.addAnimation(idle_body);
    skeleton_body_walk.addAnimation(walk_body);

    skeleton_head_idle.addAnimation(idle_head);
    skeleton_head_walk.addAnimation(walk_head);

    // Set animation speeds
    skeleton_body_idle.setAnimationSpeed("idle", animationSpeed);
    skeleton_body_walk.setAnimationSpeed("walk", animationSpeed);
    skeleton_head_idle.setAnimationSpeed("idle", animationSpeed);
    skeleton_head_walk.setAnimationSpeed("walk", animationSpeed);
}

void AnimationDemo::handleInput() {
    // Toggle between idle and walk cycle
    if (cf_key_just_pressed(CF_KEY_SPACE)) {
        if (showIdle) {
            showIdle = false;
            showWalkCycle = true;
            switchToAnimation(false);
        } else {
            showIdle = true;
            showWalkCycle = false;
            switchToAnimation(true);
        }
    }

    // Control direction with WASD
    bool directionChanged = false;
    if (cf_key_just_pressed(CF_KEY_W)) {
        currentDirection = SkeletonDirection::UP;
        directionChanged = true;
    } else if (cf_key_just_pressed(CF_KEY_A)) {
        currentDirection = SkeletonDirection::LEFT;
        directionChanged = true;
    } else if (cf_key_just_pressed(CF_KEY_S)) {
        currentDirection = SkeletonDirection::DOWN;
        directionChanged = true;
    } else if (cf_key_just_pressed(CF_KEY_D)) {
        currentDirection = SkeletonDirection::RIGHT;
        directionChanged = true;
    }

    // If direction changed, update animations
    if (directionChanged) {
        setupAnimations();
        if (showIdle) {
            switchToAnimation(true);
        } else {
            switchToAnimation(false);
        }
    }

    // Control animation speed
    if (cf_key_down(CF_KEY_UP)) {
        animationSpeed = std::min(3.0f, animationSpeed + 0.1f);
        updateAnimationSpeeds();
    }
    if (cf_key_down(CF_KEY_DOWN)) {
        animationSpeed = std::max(0.1f, animationSpeed - 0.1f);
        updateAnimationSpeeds();
    }

    // Pause/unpause animation
    if (cf_key_just_pressed(CF_KEY_P)) {
        animationPaused = !animationPaused;
        if (animationPaused) {
            if (current_body) current_body->pauseAnimation();
            if (current_head) current_head->pauseAnimation();
        } else {
            if (current_body) current_body->resumeAnimation();
            if (current_head) current_head->resumeAnimation();
        }
    }

    // Reset demo
    if (cf_key_just_pressed(CF_KEY_R)) {
        reset();
    }
}

void AnimationDemo::switchToAnimation(bool isIdle) {
    if (isIdle) {
        current_body = &skeleton_body_idle;
        current_head = &skeleton_head_idle;
        current_body->playAnimation("idle");
        current_head->playAnimation("idle");
    } else {
        current_body = &skeleton_body_walk;
        current_head = &skeleton_head_walk;
        current_body->playAnimation("walk");
        current_head->playAnimation("walk");
    }
}

void AnimationDemo::updateAnimationSpeeds() {
    skeleton_body_idle.setAnimationSpeed("idle", animationSpeed);
    skeleton_body_walk.setAnimationSpeed("walk", animationSpeed);
    skeleton_head_idle.setAnimationSpeed("idle", animationSpeed);
    skeleton_head_walk.setAnimationSpeed("walk", animationSpeed);
}

void AnimationDemo::updateAnimations(float dt) {
    if (!animationPaused) {
        demoTime += dt;

        // Update sprite animations
        if (current_body) current_body->update(dt);
        if (current_head) current_head->update(dt);
    }
}

void AnimationDemo::renderSprites() {
    // Render animated sprites
    printf("AnimationDemo::renderSprites() - Rendering sprites...\n");
    if (current_body) {
        printf("Rendering body sprite...\n");
        current_body->render();
    } else {
        printf("No body sprite to render!\n");
    }
    if (current_head) {
        printf("Rendering head sprite...\n");
        current_head->render();
    } else {
        printf("No head sprite to render!\n");
    }
    printf("AnimationDemo::renderSprites() - Sprites rendered\n");
}

void AnimationDemo::renderDemoInfo() {
    // Draw demo information in the lower-left quadrant
    v2 text_position = v2(-300, -50);
    draw_text("Animation System Demo - Phase 1.2", text_position);

    // Convert numbers to strings for display
    std::stringstream ss;

    ss.str("");
    ss << "Current Animation: " << (showIdle ? "IDLE" : "WALK CYCLE");
    draw_text(ss.str().c_str(), v2(-300, -70));

    ss.str("");
    ss << "Direction: ";
    switch (currentDirection) {
        case SkeletonDirection::UP: ss << "UP"; break;
        case SkeletonDirection::LEFT: ss << "LEFT"; break;
        case SkeletonDirection::DOWN: ss << "DOWN"; break;
        case SkeletonDirection::RIGHT: ss << "RIGHT"; break;
    }
    draw_text(ss.str().c_str(), v2(-300, -90));

    ss.str("");
    ss << "Animation Speed: " << std::fixed << std::setprecision(1) << animationSpeed << "x";
    draw_text(ss.str().c_str(), v2(-300, -110));

    ss.str("");
    ss << "Status: " << (animationPaused ? "PAUSED" : "PLAYING");
    draw_text(ss.str().c_str(), v2(-300, -130));

    ss.str("");
    if (current_body) {
        ss << "Body Frame: " << current_body->getCurrentFrameIndex() << "/" << (current_body->getAnimationCount() > 0 ? current_body->getCurrentAnimation().getFrameCount() : 0);
    } else {
        ss << "Body Frame: N/A";
    }
    draw_text(ss.str().c_str(), v2(-300, -150));

    ss.str("");
    ss << "Demo Time: " << std::fixed << std::setprecision(1) << demoTime << "s";
    draw_text(ss.str().c_str(), v2(-300, -170));

    // Controls
    draw_text("Controls:", v2(-300, -200));
    draw_text("SPACE - Toggle Idle/Walk", v2(-300, -220));
    draw_text("WASD - Change Direction", v2(-300, -240));
    draw_text("UP/DOWN - Change Speed", v2(-300, -260));
    draw_text("P - Pause/Resume", v2(-300, -280));
    draw_text("R - Reset Demo", v2(-300, -300));

    // Animation details
    if (showWalkCycle) {
        draw_text("Walk Cycle: 9 frames, 0.1s each", v2(-300, -330));
        draw_text("Sprite Sheet: 576x256 (9 frames × 4 directions)", v2(-300, -350));
        ss.str("");
        ss << "Current Direction: Row " << static_cast<int>(currentDirection);
        draw_text(ss.str().c_str(), v2(-300, -370));
    } else {
        draw_text("Idle: Single frame, 0.5s duration", v2(-300, -330));
        draw_text("Sprite Sheet: 64x256 (single frame)", v2(-300, -350));
        ss.str("");
        ss << "Current Direction: Row " << static_cast<int>(currentDirection);
        draw_text(ss.str().c_str(), v2(-300, -370));
    }
}

void AnimationDemo::update(float dt) {
    handleInput();
    updateAnimations(dt);
}

void AnimationDemo::render() {
    printf("AnimationDemo::render() - Starting render...\n");
    renderSprites();
    renderDemoInfo();
    printf("AnimationDemo::render() - Render complete\n");
}

void AnimationDemo::reset() {
    demoTime = 0.0f;
    showIdle = true;
    showWalkCycle = false;
    currentDirection = SkeletonDirection::DOWN;
    animationSpeed = 1.0f;
    animationPaused = false;

    // Reset to idle animation
    setupAnimations();
    switchToAnimation(true);
    updateAnimationSpeeds();
}
