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
    // Rebuild the JSON object from the current display lines
    m_dataFile.clear(); // Clear existing data

    for (const auto &line : m_displayLines)
    {
        std::string key(line.boxA);
        std::string value(line.boxB);

        // Skip empty keys
        if (!key.empty())
        {
            // Try to parse the value as different types
            if (value == "true" || value == "false")
            {
                m_dataFile[key] = (value == "true");
            }
            else if (value == "null")
            {
                m_dataFile[key] = nullptr;
            }
            else
            {
                // Try to parse as number
                try
                {
                    // Check if it's an integer
                    if (value.find('.') == std::string::npos)
                    {
                        int64_t intValue = std::stoll(value);
                        m_dataFile[key] = intValue;
                    }
                    else
                    {
                        double doubleValue = std::stod(value);
                        m_dataFile[key] = doubleValue;
                    }
                }
                catch (const std::exception &)
                {
                    // If parsing as number fails, treat as string
                    m_dataFile[key] = value;
                }
            }
        }
    }
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
        if (igButton("Add Line", (ImVec2){80, 0}))
        {
            // Add a new key-value pair to the JSON data
            std::string newKey = "New Key";
            std::string newValue = "New Value";

            // Ensure the key is unique by appending a number if it already exists
            int counter = 1;
            std::string uniqueKey = newKey;
            while (m_dataFile.contains(uniqueKey))
            {
                uniqueKey = newKey + " " + std::to_string(counter);
                counter++;
            }

            // Add the new key-value pair to the JSON
            m_dataFile[uniqueKey] = newValue;

            // Refresh the display from the updated JSON
            populateFromJson();
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
                // Remove the corresponding key from JSON if it exists
                std::string keyToRemove(m_displayLines[i].boxA);
                if (!keyToRemove.empty() && m_dataFile.contains(keyToRemove))
                {
                    m_dataFile.erase(keyToRemove);
                }

                m_displayLines.erase(m_displayLines.begin() + i);
                break; // Break to avoid iterator invalidation
            }
        }

        igSeparator();

        // JSON synchronization controls
        if (igButton("Update JSON from Fields", (ImVec2){150, 0}))
        {
            updateJsonText();
        }

        igSameLine(0, 10);
        if (igButton("Refresh from JSON", (ImVec2){120, 0}))
        {
            populateFromJson();
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