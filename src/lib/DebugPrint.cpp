#include "DebugPrint.h"
#include <cstdio>
#include <cstdarg>
#include <algorithm>
#include <cstring>

namespace DebugPrint
{
    // ANSI color codes for terminal output
    static const char *s_colors[] = {
        "\033[31m", // Red
        "\033[32m", // Green
        "\033[33m", // Yellow
        "\033[34m", // Blue
        "\033[35m", // Magenta
        "\033[36m", // Cyan
        "\033[91m", // Bright Red
        "\033[92m", // Bright Green
        "\033[93m", // Bright Yellow
        "\033[94m", // Bright Blue
        "\033[95m", // Bright Magenta
        "\033[96m", // Bright Cyan
    };
    static const int s_numColors = sizeof(s_colors) / sizeof(s_colors[0]);
    static const char *s_resetColor = "\033[0m";

    // Use fixed-size arrays to avoid dynamic allocation
    static constexpr int MAX_CHANNELS = 64;
    static constexpr int MAX_CHANNEL_NAME_LEN = 64;

    struct ChannelInfo
    {
        char name[MAX_CHANNEL_NAME_LEN];
        int colorIndex;
        bool enabled;
    };

    static ChannelInfo s_channels[MAX_CHANNELS];
    static int s_numChannels = 0;
    static bool s_allChannelsEnabled = false;

    void Init()
    {
        s_numChannels = 0;
        s_allChannelsEnabled = false;
        for (int i = 0; i < MAX_CHANNELS; i++)
        {
            s_channels[i].name[0] = '\0';
            s_channels[i].colorIndex = 0;
            s_channels[i].enabled = false;
        }
    }

    // Find channel index, returns -1 if not found
    static int FindChannel(const char *channel)
    {
        for (int i = 0; i < s_numChannels; i++)
        {
            if (strcmp(s_channels[i].name, channel) == 0)
            {
                return i;
            }
        }
        return -1;
    }

    void RegisterChannel(const char *channel)
    {
        if (FindChannel(channel) >= 0)
        {
            return; // Already registered
        }

        if (s_numChannels >= MAX_CHANNELS)
        {
            return; // Too many channels
        }

        strncpy(s_channels[s_numChannels].name, channel, MAX_CHANNEL_NAME_LEN - 1);
        s_channels[s_numChannels].name[MAX_CHANNEL_NAME_LEN - 1] = '\0';
        s_channels[s_numChannels].colorIndex = s_numChannels % s_numColors;
        s_channels[s_numChannels].enabled = false;
        s_numChannels++;
    }

    // Wrapper for std::string version
    void RegisterChannel(const std::string &channel)
    {
        RegisterChannel(channel.c_str());
    }

    static const char *GetChannelColor(const char *channel)
    {
        int idx = FindChannel(channel);
        if (idx >= 0)
        {
            return s_colors[s_channels[idx].colorIndex];
        }
        return s_colors[0]; // Default to first color
    }

    std::vector<std::string> GetRegisteredChannels()
    {
        std::vector<std::string> result;
        result.reserve(s_numChannels);
        for (int i = 0; i < s_numChannels; i++)
        {
            result.push_back(s_channels[i].name);
        }
        return result;
    }

    void EnableChannel(const std::string &channel)
    {
        int idx = FindChannel(channel.c_str());
        if (idx >= 0)
        {
            s_channels[idx].enabled = true;
        }
    }

    void DisableChannel(const std::string &channel)
    {
        int idx = FindChannel(channel.c_str());
        if (idx >= 0)
        {
            s_channels[idx].enabled = false;
        }
    }

    static bool IsChannelEnabled(const char *channel)
    {
        if (s_allChannelsEnabled)
        {
            return true;
        }
        int idx = FindChannel(channel);
        if (idx >= 0)
        {
            return s_channels[idx].enabled;
        }
        return false;
    }

    bool IsChannelEnabled(const std::string &channel)
    {
        return IsChannelEnabled(channel.c_str());
    }

    void EnableAllChannels()
    {
        s_allChannelsEnabled = true;
    }

    void DisableAllChannels()
    {
        s_allChannelsEnabled = false;
    }

    void Print(const char *channel, const char *format, ...)
    {
        // Auto-register channel on first use
        RegisterChannel(channel);

        if (!IsChannelEnabled(channel))
        {
            return;
        }

        // Print colored channel prefix
        printf("%s[%s]%s ", GetChannelColor(channel), channel, s_resetColor);

        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
    }

    void Print(const char *format, ...)
    {
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
    }

    void VPrint(const char *channel, const char *format, va_list args)
    {
        // Auto-register channel on first use
        RegisterChannel(channel);

        if (!IsChannelEnabled(channel))
        {
            return;
        }

        // Print colored channel prefix
        printf("%s[%s]%s ", GetChannelColor(channel), channel, s_resetColor);

        vprintf(format, args);
    }

    void VPrint(const char *format, va_list args)
    {
        vprintf(format, args);
    }
}
