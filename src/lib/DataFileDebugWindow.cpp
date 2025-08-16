#include "DataFileDebugWindow.h"
#include <stdio.h>
#include <cimgui.h>
#include <algorithm>

DataFileDebugWindow::DataFileDebugWindow(const std::string &title, DataFile &dataFile)
    : DebugWindow(title), m_dataFile(dataFile)
{
    // Populate display lines from JSON data on construction
    populateFromJson();
}

void DataFileDebugWindow::addDisplayLine(const std::string &key, const std::string &value, JsonType type, int indentLevel, const std::string &parentPath)
{
    m_displayLines.emplace_back(key, value, type, indentLevel, parentPath);
}

const char *DataFileDebugWindow::getTypeString(JsonType type)
{
    switch (type)
    {
    case JsonType::String:
        return "String";
    case JsonType::Map:
        return "Map";
    case JsonType::List:
        return "List";
    case JsonType::Float:
        return "Float";
    case JsonType::Boolean:
        return "Boolean";
    case JsonType::Integer:
        return "Integer";
    default:
        return "String";
    }
}

JsonType DataFileDebugWindow::getTypeFromJson(const nlohmann::json &value)
{
    if (value.is_string())
        return JsonType::String;
    if (value.is_object())
        return JsonType::Map;
    if (value.is_array())
        return JsonType::List;
    if (value.is_number_float())
        return JsonType::Float;
    if (value.is_boolean())
        return JsonType::Boolean;
    if (value.is_number_integer())
        return JsonType::Integer;
    return JsonType::String;
}

void DataFileDebugWindow::renderTypeCombo(const char *label, JsonType *type)
{
    const char *typeNames[] = {"String", "Map", "List", "Float", "Boolean", "Integer"};
    int currentType = static_cast<int>(*type);

    if (igCombo_Str_arr(label, &currentType, typeNames, 6, -1))
    {
        *type = static_cast<JsonType>(currentType);
    }
}

std::string DataFileDebugWindow::getJsonPath(const std::string &parentPath, const std::string &key)
{
    if (parentPath.empty())
        return key;
    return parentPath + "." + key;
}

void DataFileDebugWindow::updateJsonFromLines()
{
    // Clear the existing JSON data
    m_dataFile.clear();

    // Build the JSON structure from display lines
    buildJsonFromLines(m_dataFile, "", 0);
}

void DataFileDebugWindow::buildJsonFromLines(nlohmann::json &jsonObj, const std::string &currentPath, int currentIndent)
{
    for (size_t i = 0; i < m_displayLines.size(); ++i)
    {
        const DisplayLine &line = m_displayLines[i];

        // Only process lines at the current indent level
        if (line.indentLevel != currentIndent)
            continue;

        // Check if this line belongs to the current path
        if (line.parentPath != currentPath)
            continue;

        std::string key(line.key);
        std::string value(line.value);

        if (key.empty())
            continue;

        // Handle different types
        switch (line.type)
        {
        case JsonType::String:
            jsonObj[key] = value;
            break;

        case JsonType::Boolean:
            jsonObj[key] = (value == "true");
            break;

        case JsonType::Integer:
            try
            {
                jsonObj[key] = std::stoll(value);
            }
            catch (...)
            {
                jsonObj[key] = 0;
            }
            break;

        case JsonType::Float:
            try
            {
                jsonObj[key] = std::stod(value);
            }
            catch (...)
            {
                jsonObj[key] = 0.0;
            }
            break;

        case JsonType::Map:
        {
            jsonObj[key] = nlohmann::json::object();
            std::string newPath = getJsonPath(currentPath, key);
            buildJsonFromLines(jsonObj[key], newPath, currentIndent + 1);
        }
        break;

        case JsonType::List:
        {
            jsonObj[key] = nlohmann::json::array();
            std::string newPath = getJsonPath(currentPath, key);
            buildJsonArrayFromLines(jsonObj[key], newPath, currentIndent + 1);
        }
        break;
        }
    }
}

void DataFileDebugWindow::buildJsonArrayFromLines(nlohmann::json &jsonArray, const std::string &currentPath, int currentIndent)
{
    // Collect all array items for this path
    std::vector<std::pair<int, const DisplayLine *>> arrayItems;

    for (size_t i = 0; i < m_displayLines.size(); ++i)
    {
        const DisplayLine &line = m_displayLines[i];

        // Only process lines at the current indent level
        if (line.indentLevel != currentIndent)
            continue;

        // Check if this line belongs to the current path
        if (line.parentPath != currentPath)
            continue;

        // Extract array index from key like "[0]", "[1]", etc.
        std::string key(line.key);
        if (key.front() == '[' && key.back() == ']')
        {
            try
            {
                int index = std::stoi(key.substr(1, key.length() - 2));
                arrayItems.push_back({index, &line});
            }
            catch (...)
            {
                // Invalid index format, skip
            }
        }
    }

    // Sort by index
    std::sort(arrayItems.begin(), arrayItems.end(),
              [](const auto &a, const auto &b)
              { return a.first < b.first; });

    // Build array elements
    for (const auto &[index, linePtr] : arrayItems)
    {
        const DisplayLine &line = *linePtr;
        std::string value(line.value);

        // Ensure array is large enough
        while ((int)jsonArray.size() <= index)
        {
            jsonArray.push_back(nullptr);
        }

        switch (line.type)
        {
        case JsonType::String:
            jsonArray[index] = value;
            break;

        case JsonType::Boolean:
            jsonArray[index] = (value == "true");
            break;

        case JsonType::Integer:
            try
            {
                jsonArray[index] = std::stoll(value);
            }
            catch (...)
            {
                jsonArray[index] = 0;
            }
            break;

        case JsonType::Float:
            try
            {
                jsonArray[index] = std::stod(value);
            }
            catch (...)
            {
                jsonArray[index] = 0.0;
            }
            break;

        case JsonType::Map:
        {
            jsonArray[index] = nlohmann::json::object();
            std::string newPath = currentPath + "." + std::to_string(index);
            buildJsonFromLines(jsonArray[index], newPath, currentIndent + 1);
        }
        break;

        case JsonType::List:
        {
            jsonArray[index] = nlohmann::json::array();
            std::string newPath = currentPath + "." + std::to_string(index);
            buildJsonArrayFromLines(jsonArray[index], newPath, currentIndent + 1);
        }
        break;
        }
    }
}

void DataFileDebugWindow::populateFromJson()
{
    m_displayLines.clear();
    populateFromJsonRecursive(m_dataFile, "", 0);
}

void DataFileDebugWindow::populateFromJsonRecursive(const nlohmann::json &jsonObj, const std::string &basePath, int indentLevel)
{
    try
    {
        if (jsonObj.is_object())
        {
            for (const auto &[key, value] : jsonObj.items())
            {
                std::string currentPath = getJsonPath(basePath, key);
                JsonType type = getTypeFromJson(value);

                if (value.is_object())
                {
                    addDisplayLine(key, "{}", JsonType::Map, indentLevel, basePath);
                    populateFromJsonRecursive(value, currentPath, indentLevel + 1);
                }
                else if (value.is_array())
                {
                    addDisplayLine(key, "[]", JsonType::List, indentLevel, basePath);
                    // For arrays, we'll show indices as keys
                    for (size_t i = 0; i < value.size(); ++i)
                    {
                        std::string indexKey = "[" + std::to_string(i) + "]";
                        JsonType itemType = getTypeFromJson(value[i]);

                        if (value[i].is_object())
                        {
                            addDisplayLine(indexKey, "{}", JsonType::Map, indentLevel + 1, currentPath);
                            populateFromJsonRecursive(value[i], currentPath + "." + std::to_string(i), indentLevel + 2);
                        }
                        else if (value[i].is_array())
                        {
                            addDisplayLine(indexKey, "[]", JsonType::List, indentLevel + 1, currentPath);
                            populateFromJsonRecursive(value[i], currentPath + "." + std::to_string(i), indentLevel + 2);
                        }
                        else
                        {
                            std::string valueStr = getValueString(value[i]);
                            addDisplayLine(indexKey, valueStr, itemType, indentLevel + 1, currentPath);
                        }
                    }
                }
                else
                {
                    std::string valueStr = getValueString(value);
                    addDisplayLine(key, valueStr, type, indentLevel, basePath);
                }
            }
        }
        else if (jsonObj.is_array())
        {
            for (size_t i = 0; i < jsonObj.size(); ++i)
            {
                std::string indexKey = "[" + std::to_string(i) + "]";
                JsonType type = getTypeFromJson(jsonObj[i]);

                if (jsonObj[i].is_object())
                {
                    addDisplayLine(indexKey, "{}", JsonType::Map, indentLevel, basePath);
                    populateFromJsonRecursive(jsonObj[i], basePath + "." + std::to_string(i), indentLevel + 1);
                }
                else if (jsonObj[i].is_array())
                {
                    addDisplayLine(indexKey, "[]", JsonType::List, indentLevel, basePath);
                    populateFromJsonRecursive(jsonObj[i], basePath + "." + std::to_string(i), indentLevel + 1);
                }
                else
                {
                    std::string valueStr = getValueString(jsonObj[i]);
                    addDisplayLine(indexKey, valueStr, type, indentLevel, basePath);
                }
            }
        }
        else
        {
            // Single value
            std::string valueStr = getValueString(jsonObj);
            JsonType type = getTypeFromJson(jsonObj);
            addDisplayLine("value", valueStr, type, 0, "");
        }
    }
    catch (const std::exception &e)
    {
        addDisplayLine("Error", std::string("Failed to parse JSON: ") + e.what(), JsonType::String, 0, "");
    }
}

std::string DataFileDebugWindow::getValueString(const nlohmann::json &value)
{
    if (value.is_string())
        return value.get<std::string>();
    else if (value.is_number_integer())
        return std::to_string(value.get<int64_t>());
    else if (value.is_number_unsigned())
        return std::to_string(value.get<uint64_t>());
    else if (value.is_number_float())
        return std::to_string(value.get<double>());
    else if (value.is_boolean())
        return value.get<bool>() ? "true" : "false";
    else if (value.is_null())
        return "null";
    else
        return value.dump();
}

void DataFileDebugWindow::render()
{
    if (m_show)
    {
        igBegin(m_title.c_str(), &m_show, 0);

        // Display path if available
        const std::string &path = m_dataFile.getpath();
        if (!path.empty())
        {
            igText("File: %s", path.c_str());
        }
        else
        {
            igText("File: <no file loaded>");
        }

        igSeparator();

        // Control buttons
        if (igButton("Add Root Item", (ImVec2){120, 0}))
        {
            addDisplayLine("NewKey", "NewValue", JsonType::String, 0, "");
        }

        igSeparator();
        igText("JSON Structure:");

        // Render all display lines with nesting support
        for (size_t i = 0; i < m_displayLines.size(); ++i)
        {
            DisplayLine &line = m_displayLines[i];

            // Create unique IDs for each row
            char labelKey[64], labelValue[64], labelType[64], labelRemove[64], labelAdd[64];
            snprintf(labelKey, sizeof(labelKey), "##Key%zu", i);
            snprintf(labelValue, sizeof(labelValue), "##Value%zu", i);
            snprintf(labelType, sizeof(labelType), "##Type%zu", i);
            snprintf(labelRemove, sizeof(labelRemove), "Remove##%zu", i);
            snprintf(labelAdd, sizeof(labelAdd), "Add##%zu", i);

            // Add indentation
            for (int indent = 0; indent < line.indentLevel; ++indent)
            {
                igIndent(20.0f);
            }

            // Row number and expand/collapse for containers
            if (line.type == JsonType::Map || line.type == JsonType::List)
            {
                if (igButton(line.isExpanded ? "-" : "+", (ImVec2){20, 0}))
                {
                    line.isExpanded = !line.isExpanded;
                }
                igSameLine(0, 5);
            }
            else
            {
                igIndent(25.0f);
            }

            // Key input (limited width)
            igPushItemWidth(150);
            igInputText(labelKey, line.key, sizeof(line.key), 0, NULL, NULL);
            igPopItemWidth();

            igSameLine(0, 10);

            // Type selection dropdown
            igPushItemWidth(80);
            renderTypeCombo(labelType, &line.type);
            igPopItemWidth();

            igSameLine(0, 10);

            // Value input (conditional based on type)
            igPushItemWidth(200);
            if (line.type == JsonType::Map)
            {
                igInputText(labelValue, line.value, sizeof(line.value), ImGuiInputTextFlags_ReadOnly, NULL, NULL);
                strcpy(line.value, "{}");
            }
            else if (line.type == JsonType::List)
            {
                igInputText(labelValue, line.value, sizeof(line.value), ImGuiInputTextFlags_ReadOnly, NULL, NULL);
                strcpy(line.value, "[]");
            }
            else if (line.type == JsonType::Boolean)
            {
                bool boolValue = (strcmp(line.value, "true") == 0);
                if (igCheckbox(labelValue, &boolValue))
                {
                    strcpy(line.value, boolValue ? "true" : "false");
                }
            }
            else
            {
                // Regular text input for String, Float, Integer
                igInputText(labelValue, line.value, sizeof(line.value), 0, NULL, NULL);
            }
            igPopItemWidth();

            igSameLine(0, 10);

            // Add child button for containers
            if (line.type == JsonType::Map || line.type == JsonType::List)
            {
                if (igButton(labelAdd, (ImVec2){50, 0}))
                {
                    std::string currentPath = getJsonPath(line.parentPath, std::string(line.key));

                    // Find the insertion point - after all children of this container
                    size_t insertIndex = i + 1;
                    while (insertIndex < m_displayLines.size() &&
                           m_displayLines[insertIndex].indentLevel > line.indentLevel)
                    {
                        insertIndex++;
                    }

                    // Create the new display line
                    DisplayLine newLine;
                    if (line.type == JsonType::Map)
                    {
                        newLine = DisplayLine("NewKey", "NewValue", JsonType::String, line.indentLevel + 1, currentPath);
                    }
                    else // List
                    {
                        // Count existing array items to get the next index
                        int arrayCount = 0;
                        for (size_t j = i + 1; j < insertIndex; ++j)
                        {
                            if (m_displayLines[j].indentLevel == line.indentLevel + 1 &&
                                m_displayLines[j].parentPath == currentPath)
                            {
                                arrayCount++;
                            }
                        }
                        std::string indexKey = "[" + std::to_string(arrayCount) + "]";
                        newLine = DisplayLine(indexKey, "NewValue", JsonType::String, line.indentLevel + 1, currentPath);
                    }

                    // Insert at the correct position
                    m_displayLines.insert(m_displayLines.begin() + insertIndex, newLine);
                }
                igSameLine(0, 5);
            }

            // Remove button
            if (igButton(labelRemove, (ImVec2){60, 0}))
            {
                m_displayLines.erase(m_displayLines.begin() + i);
                // Also remove any children
                while (i < m_displayLines.size() && m_displayLines[i].indentLevel > line.indentLevel)
                {
                    m_displayLines.erase(m_displayLines.begin() + i);
                }

                // Reset indentation
                for (int indent = 0; indent <= line.indentLevel; ++indent)
                {
                    igUnindent(20.0f);
                }
                if (line.type != JsonType::Map && line.type != JsonType::List)
                {
                    igUnindent(25.0f);
                }
                break; // Break to avoid iterator invalidation
            }

            // Reset indentation for this line
            for (int indent = 0; indent < line.indentLevel; ++indent)
            {
                igUnindent(20.0f);
            }
            if (line.type != JsonType::Map && line.type != JsonType::List)
            {
                igUnindent(25.0f);
            }
        }

        igSeparator();

        // JSON synchronization controls
        if (igButton("Update JSON from Fields", (ImVec2){150, 0}))
        {
            updateJsonFromLines();
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