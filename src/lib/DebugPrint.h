#pragma once
#include <cstdarg>
#include <cstdio>
#include <string>
#include <unordered_set>
#include <vector>

// Debug print utility with optional channel filtering
// Channels can be enabled/disabled to control which messages are printed

namespace DebugPrint
{
    // Initialize the debug print system
    void Init();

    // Register a channel (tracks it for UI display, auto-enables it)
    void RegisterChannel(const std::string &channel);
    void RegisterChannel(const char *channel);

    // Get all registered channels (returns a copy to avoid allocation issues)
    std::vector<std::string> GetRegisteredChannels();

    // Enable a specific channel (messages on this channel will be printed)
    void EnableChannel(const std::string &channel);

    // Disable a specific channel (messages on this channel will be suppressed)
    void DisableChannel(const std::string &channel);

    // Check if a channel is enabled
    bool IsChannelEnabled(const std::string &channel);

    // Enable all channels (default behavior)
    void EnableAllChannels();

    // Disable all channels
    void DisableAllChannels();

    // Print a message to a specific channel (only prints if channel is enabled)
    // Usage: DebugPrint::Print("MyChannel", "Value: %d\n", value);
    void Print(const char *channel, const char *format, ...);

    // Print a message with no channel (always prints, like regular printf)
    // Usage: DebugPrint::Print("Always printed: %d\n", value);
    void Print(const char *format, ...);

    // Variadic versions that take va_list
    void VPrint(const char *channel, const char *format, va_list args);
    void VPrint(const char *format, va_list args);
}

// Convenience macros for debug printing
#define DEBUG_PRINT(channel, ...) DebugPrint::Print(channel, __VA_ARGS__)
#define DEBUG_PRINT_ALWAYS(...) DebugPrint::Print(__VA_ARGS__)
