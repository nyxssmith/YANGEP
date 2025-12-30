#include "DebugStateWindow.h"
#include "State.h"
#include "DataFile.h"
#include <nlohmann/json.hpp>

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

    ImGui_Begin(m_title.c_str(), &m_show, 0);

    // Get the state's default values
    const DataFile &defaultValues = m_state->getDefaultValues();

    // Display state running status
    ImGui_Text("Is Running: %s", m_state->getIsRunning() ? "Yes" : "No");
    ImGui_Separator();

    // Display all key-value pairs from the state's data
    ImGui_Text("State Values:");
    ImGui_Indent();

    if (defaultValues.empty())
    {
        ImGui_Text("(No values)");
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
                ImGui_Text("%s: %s", key.c_str(), value.get<std::string>().c_str());
            }
            else if (value.is_number_integer())
            {
                ImGui_Text("%s: %lld", key.c_str(), value.get<long long>());
            }
            else if (value.is_number_float())
            {
                ImGui_Text("%s: %.3f", key.c_str(), value.get<double>());
            }
            else if (value.is_boolean())
            {
                ImGui_Text("%s: %s", key.c_str(), value.get<bool>() ? "true" : "false");
            }
            else if (value.is_null())
            {
                ImGui_Text("%s: null", key.c_str());
            }
            else if (value.is_array())
            {
                ImGui_Text("%s: [array with %zu elements]", key.c_str(), value.size());
            }
            else if (value.is_object())
            {
                ImGui_Text("%s: {object with %zu keys}", key.c_str(), value.size());
            }
            else
            {
                ImGui_Text("%s: (unknown type)", key.c_str());
            }
        }
    }

    ImGui_Unindent();

    ImGui_End();
}

bool DebugStateWindow::isTracking(const State *state) const
{
    return m_state == state;
}

State *DebugStateWindow::getState() const
{
    return m_state;
}
