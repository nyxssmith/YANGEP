#include "DataFileDebugWindow.h"
#include <stdio.h>

DataFileDebugWindow::DataFileDebugWindow(const std::string &title, DataFile &dataFile)
    : DebugWindow(title), m_dataFile(dataFile)
{
    // Populate display lines from JSON data on construction
    populateFromJson();
}

void DataFileDebugWindow::addDisplayLine(const std::string &boxA, const std::string &boxB)
{
    m_displayLines.emplace_back(boxA, boxB);
}

void DataFileDebugWindow::updateJsonText()
{
    // Implementation for updateJsonText if needed
}

void DataFileDebugWindow::populateFromJson()
{
    // Clear existing display lines
    m_displayLines.clear();

    try
    {
        // Check if the DataFile is an object (key-value pairs)
        if (m_dataFile.is_object())
        {
            // Iterate through each key-value pair in the JSON object
            for (const auto &[key, value] : m_dataFile.items())
            {
                std::string valueStr;

                // Convert the value to string based on its type
                if (value.is_string())
                {
                    valueStr = value.get<std::string>();
                }
                else if (value.is_number_integer())
                {
                    valueStr = std::to_string(value.get<int64_t>());
                }
                else if (value.is_number_unsigned())
                {
                    valueStr = std::to_string(value.get<uint64_t>());
                }
                else if (value.is_number_float())
                {
                    valueStr = std::to_string(value.get<double>());
                }
                else if (value.is_boolean())
                {
                    valueStr = value.get<bool>() ? "true" : "false";
                }
                else if (value.is_null())
                {
                    valueStr = "null";
                }
                else
                {
                    // For complex types (arrays, objects), use dump() to get JSON representation
                    valueStr = value.dump();
                }

                // Add a display line with the key and value
                addDisplayLine(key, valueStr);
            }
        }
        else
        {
            // If it's not an object, add a single line showing the type and value
            std::string typeStr = "Non-object JSON";
            std::string valueStr = m_dataFile.dump();
            addDisplayLine(typeStr, valueStr);
        }
    }
    catch (const std::exception &e)
    {
        // If there's an error, add an error display line
        addDisplayLine("Error", std::string("Failed to parse JSON: ") + e.what());
    }
}

void DataFileDebugWindow::render()
{
    if (m_show)
    {
        igBegin(m_title.c_str(), &m_show, 0);

        // Display filename if available
        const std::string &filename = m_dataFile.getFilename();
        if (!filename.empty())
        {
            igText("File: %s", filename.c_str());
        }
        else
        {
            igText("File: <no file loaded>");
        }

        igSeparator();

        // Control buttons
        if (igButton("Refresh from JSON", (ImVec2){120, 0}))
        {
            populateFromJson();
        }

        igSameLine(0, 10);
        if (igButton("Add Line", (ImVec2){80, 0}))
        {
            addDisplayLine("New Key", "New Value");
        }

        igSeparator();
        igText("Key-Value Pairs:");

        // Render all display lines
        for (size_t i = 0; i < m_displayLines.size(); ++i)
        {
            // Create unique IDs for each row
            char labelA[32], labelB[32], labelRemove[32];
            snprintf(labelA, sizeof(labelA), "##Key%zu", i);
            snprintf(labelB, sizeof(labelB), "##Value%zu", i);
            snprintf(labelRemove, sizeof(labelRemove), "Remove##%zu", i);

            // Label for the row
            igText("%zu:", i + 1);
            igSameLine(0, 5);

            // Key input box
            igInputText(labelA, m_displayLines[i].boxA, sizeof(m_displayLines[i].boxA), 0, NULL, NULL);

            igSameLine(0, 10);

            // Value input box
            igInputText(labelB, m_displayLines[i].boxB, sizeof(m_displayLines[i].boxB), 0, NULL, NULL);

            // Remove button
            igSameLine(0, 10);
            if (igButton(labelRemove, (ImVec2){60, 0}))
            {
                m_displayLines.erase(m_displayLines.begin() + i);
                break; // Break to avoid iterator invalidation
            }
        }

        igSeparator();

        // Bottom controls
        if (igButton("Save to File", (ImVec2){100, 0}))
        {
            if (m_dataFile.save())
            {
                printf("DataFile saved successfully\n");
            }
            else
            {
                printf("Failed to save DataFile\n");
            }
        }

        igSameLine(0, 10);
        if (igButton("Reload from File", (ImVec2){120, 0}))
        {
            if (m_dataFile.load())
            {
                populateFromJson();
                printf("DataFile reloaded successfully\n");
            }
            else
            {
                printf("Failed to reload DataFile\n");
            }
        }

        igEnd();
    }
}