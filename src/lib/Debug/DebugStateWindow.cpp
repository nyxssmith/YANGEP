#include "DebugStateWindow.h"
#include "State.h"
#include "DataFile.h"
#include <nlohmann/json.hpp>
#include <imgui.h>

DebugStateWindow::DebugStateWindow(const std::string &title, State *state)
    : DebugWindow(title), m_state(state)
{
}

void DebugStateWindow::render()
{
    if (!m_show || !m_state)
    {
        return;
    }

    ImGui::Begin(m_title.c_str(), &m_show);

    // Get the state's default values
    const DataFile &defaultValues = m_state->getDefaultValues();

    // Display state running status
    ImGui::Text("Is Running: %s", m_state->getIsRunning() ? "Yes" : "No");
    ImGui::Separator();

    // Display all key-value pairs from the state's data
    ImGui::Text("State Values:");
    ImGui::Indent();

    if (defaultValues.empty())
    {
        ImGui::Text("(No values)");
    }
    else
    {
        // Iterate through all items in the JSON
        for (auto it = defaultValues.begin(); it != defaultValues.end(); ++it)
        {
            const std::string &key = it.key();
            const auto &value = it.value();

            // Display different types appropriately
            if (value.is_string())
            {
                ImGui::Text("%s: %s", key.c_str(), value.get<std::string>().c_str());
            }
            else if (value.is_number_integer())
            {
                ImGui::Text("%s: %lld", key.c_str(), value.get<long long>());
            }
            else if (value.is_number_float())
            {
                ImGui::Text("%s: %.3f", key.c_str(), value.get<double>());
            }
            else if (value.is_boolean())
            {
                ImGui::Text("%s: %s", key.c_str(), value.get<bool>() ? "true" : "false");
            }
            else if (value.is_null())
            {
                ImGui::Text("%s: null", key.c_str());
            }
            else if (value.is_array())
            {
                ImGui::Text("%s: [array with %zu elements]", key.c_str(), value.size());
            }
            else if (value.is_object())
            {
                ImGui::Text("%s: {object with %zu keys}", key.c_str(), value.size());
            }
            else
            {
                ImGui::Text("%s: (unknown type)", key.c_str());
            }
        }
    }

    ImGui::Unindent();

    ImGui::End();
}

bool DebugStateWindow::isTracking(const State *state) const
{
    return m_state == state;
}

State *DebugStateWindow::getState() const
{
    return m_state;
}
