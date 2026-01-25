#include "DebugInputInfoWindow.h"
#include <dcimgui.h>

using namespace Cute;

DebugInputInfoWindow::DebugInputInfoWindow(const std::string &title)
    : DebugWindow(title)
{
}

void DebugInputInfoWindow::render()
{
    if (m_show)
    {
        ImGui_Begin(m_title.c_str(), &m_show, 0);

        if (ImGui_CollapsingHeader("Movement", ImGuiTreeNodeFlags_DefaultOpen))
        {
            renderMovementSection();
        }

        if (ImGui_CollapsingHeader("Keyboard", ImGuiTreeNodeFlags_DefaultOpen))
        {
            renderKeyboardSection();
        }

        if (ImGui_CollapsingHeader("Mouse", ImGuiTreeNodeFlags_DefaultOpen))
        {
            renderMouseSection();
        }

        if (ImGui_CollapsingHeader("Joystick/Controller", ImGuiTreeNodeFlags_DefaultOpen))
        {
            renderJoystickSection();
        }

        ImGui_End();
    }
}

void DebugInputInfoWindow::renderKeyboardSection()
{
    ImGui_Indent();

    // Common keys to display
    struct KeyInfo
    {
        CF_KeyButton key;
        const char *name;
    };

    KeyInfo keys[] = {
        {CF_KEY_W, "W"}, {CF_KEY_A, "A"}, {CF_KEY_S, "S"}, {CF_KEY_D, "D"}, {CF_KEY_SPACE, "Space"}, {CF_KEY_ESCAPE, "Esc"}, {CF_KEY_RETURN, "Enter"}, {CF_KEY_LSHIFT, "LShift"}, {CF_KEY_RSHIFT, "RShift"}, {CF_KEY_LCTRL, "LCtrl"}, {CF_KEY_RCTRL, "RCtrl"}, {CF_KEY_LALT, "LAlt"}, {CF_KEY_RALT, "RAlt"}, {CF_KEY_UP, "Up"}, {CF_KEY_DOWN, "Down"}, {CF_KEY_LEFT, "Left"}, {CF_KEY_RIGHT, "Right"}, {CF_KEY_Q, "Q"}, {CF_KEY_E, "E"}, {CF_KEY_R, "R"}, {CF_KEY_F, "F"}, {CF_KEY_1, "1"}, {CF_KEY_2, "2"}, {CF_KEY_3, "3"}, {CF_KEY_4, "4"}};

    ImGui_Text("Pressed Keys:");
    ImGui_Indent();
    bool anyPressed = false;
    for (const auto &keyInfo : keys)
    {
        if (cf_key_down(keyInfo.key))
        {
            ImGui_TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%s", keyInfo.name);
            ImGui_SameLine();
            anyPressed = true;
        }
    }
    if (!anyPressed)
    {
        ImGui_Text("(none)");
    }
    else
    {
        ImGui_NewLine();
    }
    ImGui_Unindent();

    ImGui_Separator();

    // Show recently pressed/released keys
    ImGui_Text("Just Pressed:");
    ImGui_Indent();
    bool anyJustPressed = false;
    for (const auto &keyInfo : keys)
    {
        if (cf_key_just_pressed(keyInfo.key))
        {
            ImGui_TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%s", keyInfo.name);
            ImGui_SameLine();
            anyJustPressed = true;
        }
    }
    if (!anyJustPressed)
    {
        ImGui_Text("(none)");
    }
    else
    {
        ImGui_NewLine();
    }
    ImGui_Unindent();

    ImGui_Unindent();
}

void DebugInputInfoWindow::renderMouseSection()
{
    ImGui_Indent();

    // Mouse position
    float mouseX = cf_mouse_x();
    float mouseY = cf_mouse_y();
    ImGui_Text("Position: (%.1f, %.1f)", mouseX, mouseY);

    // Mouse wheel
    int mouseWheel = cf_mouse_wheel_motion();
    ImGui_Text("Wheel: %d", mouseWheel);

    ImGui_Separator();

    // Mouse buttons
    ImGui_Text("Buttons:");
    ImGui_Indent();

    if (cf_mouse_down(CF_MOUSE_BUTTON_LEFT))
    {
        ImGui_TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Left");
    }
    else
    {
        ImGui_Text("Left");
    }

    if (cf_mouse_down(CF_MOUSE_BUTTON_RIGHT))
    {
        ImGui_TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Right");
    }
    else
    {
        ImGui_Text("Right");
    }

    if (cf_mouse_down(CF_MOUSE_BUTTON_MIDDLE))
    {
        ImGui_TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Middle");
    }
    else
    {
        ImGui_Text("Middle");
    }

    ImGui_Unindent();

    // Just pressed
    ImGui_Separator();
    ImGui_Text("Just Pressed:");
    ImGui_Indent();
    bool anyPressed = false;
    if (cf_mouse_just_pressed(CF_MOUSE_BUTTON_LEFT))
    {
        ImGui_TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Left");
        anyPressed = true;
    }
    if (cf_mouse_just_pressed(CF_MOUSE_BUTTON_RIGHT))
    {
        ImGui_TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Right");
        anyPressed = true;
    }
    if (cf_mouse_just_pressed(CF_MOUSE_BUTTON_MIDDLE))
    {
        ImGui_TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Middle");
        anyPressed = true;
    }
    if (!anyPressed)
    {
        ImGui_Text("(none)");
    }
    ImGui_Unindent();

    ImGui_Unindent();
}

void DebugInputInfoWindow::renderJoystickSection()
{
    ImGui_Indent();

    // Check how many joysticks are connected
    int joystickCount = cf_joypad_count();
    ImGui_Text("Connected Controllers: %d", joystickCount);

    if (joystickCount == 0)
    {
        ImGui_Text("(no controllers detected)");
        ImGui_Unindent();
        return;
    }

    ImGui_Separator();

    // Display info for each connected joystick
    for (int i = 0; i < joystickCount; i++)
    {
        char nodeLabel[64];
        snprintf(nodeLabel, sizeof(nodeLabel), "Controller %d", i);

        if (ImGui_TreeNode(nodeLabel))
        {
            const char *name = cf_joypad_name(i);
            int powerLevel = cf_joypad_power_level(i);

            ImGui_Text("Name: %s", name ? name : "Unknown");
            ImGui_Text("Power Level: %d%%", powerLevel);

            ImGui_Separator();

            // Buttons
            ImGui_Text("Buttons:");
            ImGui_Indent();

            struct ButtonInfo
            {
                CF_JoypadButton button;
                const char *name;
            };

            ButtonInfo buttons[] = {
                {CF_JOYPAD_BUTTON_A, "A"},
                {CF_JOYPAD_BUTTON_B, "B"},
                {CF_JOYPAD_BUTTON_X, "X"},
                {CF_JOYPAD_BUTTON_Y, "Y"},
                {CF_JOYPAD_BUTTON_BACK, "Back"},
                {CF_JOYPAD_BUTTON_GUIDE, "Guide"},
                {CF_JOYPAD_BUTTON_START, "Start"},
                {CF_JOYPAD_BUTTON_LEFTSTICK, "L-Stick"},
                {CF_JOYPAD_BUTTON_RIGHTSTICK, "R-Stick"},
                {CF_JOYPAD_BUTTON_LEFTSHOULDER, "LB"},
                {CF_JOYPAD_BUTTON_RIGHTSHOULDER, "RB"},
                {CF_JOYPAD_BUTTON_DPAD_UP, "D-Up"},
                {CF_JOYPAD_BUTTON_DPAD_DOWN, "D-Down"},
                {CF_JOYPAD_BUTTON_DPAD_LEFT, "D-Left"},
                {CF_JOYPAD_BUTTON_DPAD_RIGHT, "D-Right"}};

            bool anyButtonPressed = false;
            for (const auto &btnInfo : buttons)
            {
                if (cf_joypad_button_down(i, btnInfo.button))
                {
                    ImGui_TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%s", btnInfo.name);
                    ImGui_SameLine();
                    anyButtonPressed = true;
                }
            }
            if (!anyButtonPressed)
            {
                ImGui_Text("(none)");
            }
            else
            {
                ImGui_NewLine();
            }
            ImGui_Unindent();

            ImGui_Separator();

            // Analog sticks
            float leftX = cf_joypad_axis(i, CF_JOYPAD_AXIS_LEFTX);
            float leftY = cf_joypad_axis(i, CF_JOYPAD_AXIS_LEFTY);
            float rightX = cf_joypad_axis(i, CF_JOYPAD_AXIS_RIGHTX);
            float rightY = cf_joypad_axis(i, CF_JOYPAD_AXIS_RIGHTY);

            ImGui_Text("Left Stick: (%.2f, %.2f)", leftX, leftY);
            ImGui_Text("Right Stick: (%.2f, %.2f)", rightX, rightY);

            // Triggers
            float leftTrigger = cf_joypad_axis(i, CF_JOYPAD_AXIS_TRIGGERLEFT);
            float rightTrigger = cf_joypad_axis(i, CF_JOYPAD_AXIS_TRIGGERRIGHT);

            ImGui_Text("Left Trigger: %.2f", leftTrigger);
            ImGui_Text("Right Trigger: %.2f", rightTrigger);

            ImGui_TreePop();
        }
    }

    ImGui_Unindent();
}

void DebugInputInfoWindow::renderMovementSection()
{
    ImGui_Indent();

    // Calculate the same movement logic as main.cpp
    const float playerSpeed = 200.0f;
    const float deadzone = 0.2f;

    // Check keyboard input
    bool keyW = cf_key_down(CF_KEY_W) || cf_key_down(CF_KEY_UP);
    bool keyS = cf_key_down(CF_KEY_S) || cf_key_down(CF_KEY_DOWN);
    bool keyA = cf_key_down(CF_KEY_A) || cf_key_down(CF_KEY_LEFT);
    bool keyD = cf_key_down(CF_KEY_D) || cf_key_down(CF_KEY_RIGHT);

    v2 moveVector = cf_v2(0.0f, 0.0f);

    // Keyboard movement (simplified - doesn't track last pressed)
    if (keyW)
        moveVector.y = playerSpeed;
    else if (keyS)
        moveVector.y = -playerSpeed;

    if (keyA)
        moveVector.x = -playerSpeed;
    else if (keyD)
        moveVector.x = playerSpeed;

    // Controller override
    bool controllerOverride = false;
    if (cf_joypad_count() > 0)
    {
        float rawStickX = cf_joypad_axis(0, CF_JOYPAD_AXIS_LEFTX);
        float rawStickY = cf_joypad_axis(0, CF_JOYPAD_AXIS_LEFTY);

        // Normalize from -32768..32767 to -1.0..1.0
        float leftStickX = rawStickX / 32767.0f;
        float leftStickY = rawStickY / 32767.0f;

        float stickMagnitude = sqrtf(leftStickX * leftStickX + leftStickY * leftStickY);

        ImGui_Text("Left Stick Raw: (%.0f, %.0f)", rawStickX, rawStickY);
        ImGui_Text("Left Stick Normalized: (%.3f, %.3f)", leftStickX, leftStickY);
        ImGui_Text("Stick Magnitude: %.3f (deadzone: %.2f)", stickMagnitude, deadzone);

        if (stickMagnitude > deadzone)
        {
            moveVector.x = leftStickX * playerSpeed;
            moveVector.y = leftStickY * playerSpeed;
            controllerOverride = true;
        }
    }

    ImGui_Separator();

    // Display calculated movement
    float moveMagnitude = sqrtf(moveVector.x * moveVector.x + moveVector.y * moveVector.y);

    if (moveMagnitude > 0.01f)
    {
        ImGui_TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "MOVING");
        ImGui_Text("Source: %s", controllerOverride ? "Controller" : "Keyboard");
        ImGui_Text("Move Vector: (%.1f, %.1f)", moveVector.x, moveVector.y);
        ImGui_Text("Speed: %.1f px/s", moveMagnitude);
    }
    else
    {
        ImGui_TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "NOT MOVING");
    }

    ImGui_Unindent();
}
