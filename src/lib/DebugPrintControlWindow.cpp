#include "DebugPrintControlWindow.h"
#include "DebugPrint.h"
#include <cute.h>

DebugPrintControlWindow::DebugPrintControlWindow(const std::string &title)
    : DebugWindow(title), m_allChannelsEnabled(false)
{
}

void DebugPrintControlWindow::render()
{
    if (m_show)
    {
        ImGui_Begin(m_title.c_str(), &m_show, 0);

        // Master toggle for all channels
        if (ImGui_Checkbox("Enable All Channels", &m_allChannelsEnabled))
        {
            if (m_allChannelsEnabled)
            {
                DebugPrint::EnableAllChannels();
            }
            else
            {
                DebugPrint::DisableAllChannels();
            }
        }

        ImGui_Separator();
        ImGui_Text("Channels:");

        // Get all registered channels
        const auto &channels = DebugPrint::GetRegisteredChannels();

        if (channels.empty())
        {
            ImGui_Text("  (No channels registered yet)");
        }
        else
        {
            // Display checkbox for each registered channel
            for (const auto &channel : channels)
            {
                bool enabled = DebugPrint::IsChannelEnabled(channel);
                if (ImGui_Checkbox(channel.c_str(), &enabled))
                {
                    if (enabled)
                    {
                        DebugPrint::EnableChannel(channel);
                    }
                    else
                    {
                        DebugPrint::DisableChannel(channel);
                        // If we disabled a channel, we're no longer in "all enabled" mode
                        m_allChannelsEnabled = false;
                    }
                }
            }
        }

        ImGui_Separator();
        ImGui_Text("Total channels: %zu", channels.size());

        ImGui_End();
    }
}
