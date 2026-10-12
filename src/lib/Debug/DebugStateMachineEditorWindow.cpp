#include "DebugStateMachineEditorWindow.h"
#include "StateLibrary.h"
#include <algorithm>
#include <cute.h>
#include <filesystem>
#include <imgui.h>

DebugStateMachineEditorWindow::DebugStateMachineEditorWindow(DataFile &dataFile)
    : DataFileBySchemaDebugWindow("State Machine Editor", dataFile, DataFileSchema::StateMachine)
{
    m_show = false;
    refreshAllowedStates();
    refreshAvailableFiles();
    const auto &currentJson = static_cast<const nlohmann::json &>(dataFile);
    if (currentJson.is_null() || !currentJson.contains("name"))
    {
        resetToDefault();
    }
}

DebugStateMachineEditorWindow::DebugStateMachineEditorWindow(const std::string &title, DataFile &dataFile)
    : DataFileBySchemaDebugWindow(title, dataFile, DataFileSchema::StateMachine)
{
    m_show = true;
    refreshAllowedStates();
    refreshAvailableFiles();
    const auto &currentJson = static_cast<const nlohmann::json &>(dataFile);
    if (currentJson.is_null() || !currentJson.contains("name"))
    {
        resetToDefault();
    }
}

void DebugStateMachineEditorWindow::show()
{
    refreshAllowedStates();
    refreshAvailableFiles();
    m_show = true;
}

void DebugStateMachineEditorWindow::refreshAllowedStates()
{
    StateLibrary lib;
    m_allowedStates = lib.getRegisteredStateNames();
    std::sort(m_allowedStates.begin(), m_allowedStates.end());
}

void DebugStateMachineEditorWindow::refreshAvailableFiles()
{
    m_existingFiles.clear();
    auto collectFrom = [this](const std::string &dirPath)
    {
        std::error_code err;
        for (const auto &entry : std::filesystem::directory_iterator(dirPath, err))
        {
            if (entry.is_regular_file() && entry.path().extension() == ".json")
            {
                m_existingFiles.push_back(entry.path().string());
            }
        }
    };

    collectFrom("assets/DataFiles/StateMachines");
    if (!m_levelDirectory.empty())
    {
        collectFrom(m_levelDirectory + "/DataFiles/StateMachines");
    }
    std::sort(m_existingFiles.begin(), m_existingFiles.end());
}

bool DebugStateMachineEditorWindow::loadStateMachine(const std::string &path)
{
    if (m_dataFile.load(path))
    {
        loadFromDataFile();
        return true;
    }
    return false;
}

void DebugStateMachineEditorWindow::resetToDefault()
{
    nlohmann::json def = {
        {"name", "new_state_machine"},
        {"states", nlohmann::json::array({
            {
                {"name", "wait"},
                {"inputs", {{"ms", 1000.0f}}}
            }
        })}
    };
    static_cast<nlohmann::json &>(m_dataFile) = def;
    m_dataFile.setpath("");
    loadFromDataFile();
}

bool DebugStateMachineEditorWindow::save_state_machine_to(const std::string &path)
{
    nlohmann::json content;
    if (!getValidWorking(content))
    {
        printf("DebugStateMachineEditorWindow: state machine does not satisfy the schema, not saving\n");
        return false;
    }

    const std::string name = content.value("name", "");
    if (name.empty() || name.find_first_of("/\\") != std::string::npos || name == "." || name == "..")
    {
        printf("DebugStateMachineEditorWindow: invalid state machine name '%s', not saving\n", name.c_str());
        return false;
    }

    std::string folder = path;
    while (folder.size() > 1 && folder.back() == '/')
        folder.pop_back();

    std::string writeFolder = folder;
    if (writeFolder.starts_with("/assets/"))
        writeFolder = writeFolder.substr(8);
    if (Cute::is_error(cf_fs_create_directory(writeFolder.c_str())))
    {
        printf("DebugStateMachineEditorWindow: failed to create folder '%s'\n", folder.c_str());
        return false;
    }

    DataFile file;
    static_cast<nlohmann::json &>(file) = content;
    std::string fullPath = folder + "/" + name + ".json";
    bool saved = file.save(fullPath);
    if (saved)
    {
        m_dataFile.setpath(fullPath);
        refreshAvailableFiles();
    }
    return saved;
}

nlohmann::json DebugStateMachineEditorWindow::createDefaultInputsForState(const std::string &stateName)
{
    if (stateName == "wait")
        return {{"ms", 1000.0f}};
    if (stateName == "print")
        return {{"to_print", "Hello"}};
    if (stateName == "wander_new_position")
        return {{"tiles_radius", 5}};
    if (stateName == "go_to_tile_position")
        return {{"tile_x", 0}, {"tile_y", 0}};
    if (stateName == "set")
        return {{"register", "A"}, {"value", false}};
    if (stateName == "say_in_bubble")
        return {{"to_say", "Hello!"}};
    if (stateName == "do_action")
        return {{"action", ""}};
    if (stateName == "if")
        return {{"register", "A"}, {"operation", "=="}, {"value", true}};
    if (stateName == "get_entities_in_box")
        return {{"top_left_tile_x", 0}, {"top_left_tile_y", 0}, {"bottom_right_tile_x", 5}, {"bottom_right_tile_y", 5}};
    if (stateName == "jump")
        return {{"to", 0}};

    if (stateName == "move_to_position" ||
        stateName == "do_action_from_coordinator" ||
        stateName == "get_new_position_from_coordinator" ||
        stateName == "endif" ||
        stateName == "get_distance_from_home" ||
        stateName == "get_distance_from_player")
    {
        return nullptr;
    }

    return nlohmann::json::object();
}

void DebugStateMachineEditorWindow::renderStateInputs(const std::string &stateName, nlohmann::json &stateJson, const std::string &id, nlohmann::json &allStates)
{
    if (stateName == "wait")
    {
        if (!stateJson.contains("inputs") || !stateJson["inputs"].is_object())
            stateJson["inputs"] = nlohmann::json::object();
        if (!stateJson["inputs"].contains("ms"))
            stateJson["inputs"]["ms"] = 1000.0f;

        float ms = stateJson["inputs"]["ms"].is_number() ? stateJson["inputs"]["ms"].get<float>() : 1000.0f;
        ImGui::Text("Wait Time:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(140);
        if (ImGui::DragFloat(("##ms" + id).c_str(), &ms, 50.0f, 0.0f, 600000.0f, "%.0f ms"))
        {
            stateJson["inputs"]["ms"] = ms;
        }
    }
    else if (stateName == "print")
    {
        if (!stateJson.contains("inputs") || !stateJson["inputs"].is_object())
            stateJson["inputs"] = nlohmann::json::object();
        if (!stateJson["inputs"].contains("to_print"))
            stateJson["inputs"]["to_print"] = "";

        std::string text = stateJson["inputs"]["to_print"].is_string() ? stateJson["inputs"]["to_print"].get<std::string>() : "";
        char buf[256];
        snprintf(buf, sizeof(buf), "%s", text.c_str());
        ImGui::Text("Print Text:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(260);
        if (ImGui::InputText(("##print" + id).c_str(), buf, sizeof(buf)))
        {
            stateJson["inputs"]["to_print"] = std::string(buf);
        }
    }
    else if (stateName == "wander_new_position")
    {
        if (!stateJson.contains("inputs") || !stateJson["inputs"].is_object())
            stateJson["inputs"] = nlohmann::json::object();
        if (!stateJson["inputs"].contains("tiles_radius"))
            stateJson["inputs"]["tiles_radius"] = 5;

        int radius = stateJson["inputs"]["tiles_radius"].is_number() ? stateJson["inputs"]["tiles_radius"].get<int>() : 5;
        ImGui::Text("Tile Radius:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(120);
        if (ImGui::DragInt(("##radius" + id).c_str(), &radius, 1.0f, 1, 100))
        {
            stateJson["inputs"]["tiles_radius"] = radius;
        }
    }
    else if (stateName == "move_to_position")
    {
        ImGui::TextDisabled("(No inputs required - moves agent to target position)");
    }
    else if (stateName == "go_to_tile_position")
    {
        if (!stateJson.contains("inputs") || !stateJson["inputs"].is_object())
            stateJson["inputs"] = nlohmann::json::object();
        if (!stateJson["inputs"].contains("tile_x"))
            stateJson["inputs"]["tile_x"] = 0;
        if (!stateJson["inputs"].contains("tile_y"))
            stateJson["inputs"]["tile_y"] = 0;

        int tileX = stateJson["inputs"]["tile_x"].is_number() ? stateJson["inputs"]["tile_x"].get<int>() : 0;
        int tileY = stateJson["inputs"]["tile_y"].is_number() ? stateJson["inputs"]["tile_y"].get<int>() : 0;

        ImGui::Text("Tile X:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(80);
        if (ImGui::InputInt(("##tile_x" + id).c_str(), &tileX))
        {
            stateJson["inputs"]["tile_x"] = tileX;
        }
        ImGui::SameLine();
        ImGui::Text("Tile Y:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(80);
        if (ImGui::InputInt(("##tile_y" + id).c_str(), &tileY))
        {
            stateJson["inputs"]["tile_y"] = tileY;
        }
    }
    else if (stateName == "set")
    {
        if (!stateJson.contains("inputs") || !stateJson["inputs"].is_object())
            stateJson["inputs"] = nlohmann::json::object();
        if (!stateJson["inputs"].contains("register"))
            stateJson["inputs"]["register"] = "A";

        std::string reg = stateJson["inputs"]["register"].is_string() ? stateJson["inputs"]["register"].get<std::string>() : "A";
        ImGui::Text("Register:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(100);
        const char *registers[] = {"A (bool)", "B (float)", "C (string)"};
        const char *regKeys[] = {"A", "B", "C"};
        int currentReg = (reg == "B") ? 1 : ((reg == "C") ? 2 : 0);
        if (ImGui::Combo(("##reg" + id).c_str(), &currentReg, registers, 3))
        {
            reg = regKeys[currentReg];
            stateJson["inputs"]["register"] = reg;
            if (reg == "A")
                stateJson["inputs"]["value"] = false;
            else if (reg == "B")
                stateJson["inputs"]["value"] = 0.0f;
            else if (reg == "C")
                stateJson["inputs"]["value"] = "";
        }
        ImGui::SameLine();
        ImGui::Text("Value:");
        ImGui::SameLine();
        if (reg == "A")
        {
            bool val = stateJson["inputs"].value("value", false);
            if (ImGui::Checkbox(("##val_bool" + id).c_str(), &val))
            {
                stateJson["inputs"]["value"] = val;
            }
        }
        else if (reg == "B")
        {
            float val = stateJson["inputs"].value("value", 0.0f);
            ImGui::SetNextItemWidth(120);
            if (ImGui::InputFloat(("##val_float" + id).c_str(), &val))
            {
                stateJson["inputs"]["value"] = val;
            }
        }
        else
        {
            std::string val = stateJson["inputs"].value("value", "");
            char buf[256];
            snprintf(buf, sizeof(buf), "%s", val.c_str());
            ImGui::SetNextItemWidth(180);
            if (ImGui::InputText(("##val_str" + id).c_str(), buf, sizeof(buf)))
            {
                stateJson["inputs"]["value"] = std::string(buf);
            }
        }
    }
    else if (stateName == "say_in_bubble")
    {
        if (!stateJson.contains("inputs") || !stateJson["inputs"].is_object())
            stateJson["inputs"] = nlohmann::json::object();
        if (!stateJson["inputs"].contains("to_say"))
            stateJson["inputs"]["to_say"] = "";

        std::string text = stateJson["inputs"]["to_say"].is_string() ? stateJson["inputs"]["to_say"].get<std::string>() : "";
        char buf[256];
        snprintf(buf, sizeof(buf), "%s", text.c_str());
        ImGui::Text("Bubble Text:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(260);
        if (ImGui::InputText(("##say" + id).c_str(), buf, sizeof(buf)))
        {
            stateJson["inputs"]["to_say"] = std::string(buf);
        }
    }
    else if (stateName == "do_action")
    {
        if (!stateJson.contains("inputs") || !stateJson["inputs"].is_object())
            stateJson["inputs"] = nlohmann::json::object();
        if (!stateJson["inputs"].contains("action"))
            stateJson["inputs"]["action"] = "";

        std::string action = stateJson["inputs"]["action"].is_string() ? stateJson["inputs"]["action"].get<std::string>() : "";
        char buf[128];
        snprintf(buf, sizeof(buf), "%s", action.c_str());
        ImGui::Text("Action Name:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(200);
        if (ImGui::InputText(("##act" + id).c_str(), buf, sizeof(buf)))
        {
            stateJson["inputs"]["action"] = std::string(buf);
        }
    }
    else if (stateName == "do_action_from_coordinator")
    {
        ImGui::TextDisabled("(No inputs required - action obtained from level coordinator)");
    }
    else if (stateName == "get_new_position_from_coordinator")
    {
        ImGui::TextDisabled("(No inputs required - target position obtained from coordinator)");
    }
    else if (stateName == "if")
    {
        if (!stateJson.contains("inputs") || !stateJson["inputs"].is_object())
            stateJson["inputs"] = nlohmann::json::object();

        std::string reg = stateJson["inputs"].value("register", "A");
        std::string op = stateJson["inputs"].value("operation", "==");

        ImGui::Text("Register:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(100);
        const char *registers[] = {"A (bool)", "B (float)", "C (string)"};
        const char *regKeys[] = {"A", "B", "C"};
        int currentReg = (reg == "B") ? 1 : ((reg == "C") ? 2 : 0);
        if (ImGui::Combo(("##if_reg" + id).c_str(), &currentReg, registers, 3))
        {
            reg = regKeys[currentReg];
            stateJson["inputs"]["register"] = reg;
            if (reg == "A")
                stateJson["inputs"]["value"] = false;
            else if (reg == "B")
                stateJson["inputs"]["value"] = 0.0f;
            else if (reg == "C")
                stateJson["inputs"]["value"] = "";
        }

        ImGui::SameLine();
        ImGui::Text("Op:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(70);
        const char *ops[] = {"==", "!=", "<", ">", "<=", ">="};
        int currentOp = 0;
        for (int o = 0; o < 6; ++o)
        {
            if (op == ops[o])
            {
                currentOp = o;
                break;
            }
        }
        if (ImGui::Combo(("##if_op" + id).c_str(), &currentOp, ops, 6))
        {
            stateJson["inputs"]["operation"] = ops[currentOp];
        }

        ImGui::SameLine();
        ImGui::Text("Value:");
        ImGui::SameLine();
        if (reg == "A")
        {
            bool val = stateJson["inputs"].value("value", false);
            if (ImGui::Checkbox(("##if_val_bool" + id).c_str(), &val))
            {
                stateJson["inputs"]["value"] = val;
            }
        }
        else if (reg == "B")
        {
            float val = stateJson["inputs"].value("value", 0.0f);
            ImGui::SetNextItemWidth(100);
            if (ImGui::InputFloat(("##if_val_float" + id).c_str(), &val))
            {
                stateJson["inputs"]["value"] = val;
            }
        }
        else
        {
            std::string val = stateJson["inputs"].value("value", "");
            char buf[256];
            snprintf(buf, sizeof(buf), "%s", val.c_str());
            ImGui::SetNextItemWidth(140);
            if (ImGui::InputText(("##if_val_str" + id).c_str(), buf, sizeof(buf)))
            {
                stateJson["inputs"]["value"] = std::string(buf);
            }
        }
    }
    else if (stateName == "endif")
    {
        ImGui::TextDisabled("(No inputs required - closes conditional block)");
    }
    else if (stateName == "get_distance_from_home")
    {
        ImGui::TextDisabled("(No inputs required - calculates distance and stores into register B)");
    }
    else if (stateName == "get_distance_from_player")
    {
        ImGui::TextDisabled("(No inputs required - calculates distance and stores into register B)");
    }
    else if (stateName == "get_entities_in_box")
    {
        if (!stateJson.contains("inputs") || !stateJson["inputs"].is_object())
            stateJson["inputs"] = nlohmann::json::object();

        int tlx = stateJson["inputs"].value("top_left_tile_x", 0);
        int tly = stateJson["inputs"].value("top_left_tile_y", 0);
        int brx = stateJson["inputs"].value("bottom_right_tile_x", 0);
        int bry = stateJson["inputs"].value("bottom_right_tile_y", 0);

        ImGui::Text("Top-Left Tile (X, Y):");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(65);
        if (ImGui::InputInt(("##tlx" + id).c_str(), &tlx))
            stateJson["inputs"]["top_left_tile_x"] = tlx;
        ImGui::SameLine();
        ImGui::SetNextItemWidth(65);
        if (ImGui::InputInt(("##tly" + id).c_str(), &tly))
            stateJson["inputs"]["top_left_tile_y"] = tly;

        ImGui::Text("Bottom-Right Tile (X, Y):");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(65);
        if (ImGui::InputInt(("##brx" + id).c_str(), &brx))
            stateJson["inputs"]["bottom_right_tile_x"] = brx;
        ImGui::SameLine();
        ImGui::SetNextItemWidth(65);
        if (ImGui::InputInt(("##bry" + id).c_str(), &bry))
            stateJson["inputs"]["bottom_right_tile_y"] = bry;
    }
    else if (stateName == "jump")
    {
        if (!stateJson.contains("inputs") || !stateJson["inputs"].is_object())
            stateJson["inputs"] = nlohmann::json::object();

        int targetIdx = stateJson["inputs"].value("to", 0);
        ImGui::Text("Jump Target:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(200);

        std::string currentLabel = "State " + std::to_string(targetIdx);
        if (targetIdx >= 0 && targetIdx < static_cast<int>(allStates.size()))
        {
            std::string sName = allStates[targetIdx].value("name", "?");
            currentLabel = "[" + std::to_string(targetIdx) + "] " + sName;
        }

        if (ImGui::BeginCombo(("##jump_to" + id).c_str(), currentLabel.c_str()))
        {
            for (int s = 0; s < static_cast<int>(allStates.size()); ++s)
            {
                std::string sName = allStates[s].value("name", "?");
                std::string optLabel = "[" + std::to_string(s) + "] " + sName;
                bool isSel = (s == targetIdx);
                if (ImGui::Selectable(optLabel.c_str(), isSel))
                {
                    stateJson["inputs"]["to"] = s;
                }
                if (isSel)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
    }
    else
    {
        // Custom or unmapped state - allow editing as raw JSON object
        if (!stateJson.contains("inputs") || !stateJson["inputs"].is_object())
            stateJson["inputs"] = nlohmann::json::object();

        std::string &buf = m_rawBuffers[id + ".inputs"];
        if (buf.empty())
            buf = stateJson["inputs"].dump();
        char text[512];
        snprintf(text, sizeof(text), "%s", buf.c_str());
        ImGui::Text("Custom Inputs (JSON):");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(300);
        if (ImGui::InputText(("##custom" + id).c_str(), text, sizeof(text)))
        {
            buf = text;
            auto parsed = nlohmann::json::parse(buf, nullptr, false);
            if (parsed.is_object())
                stateJson["inputs"] = parsed;
        }
    }
}

void DebugStateMachineEditorWindow::renderArrayBody(nlohmann::json &value, const nlohmann::json &schema, const std::string &id)
{
    if (id != "root.states")
    {
        DataFileBySchemaDebugWindow::renderArrayBody(value, schema, id);
        return;
    }

    if (!value.is_array())
    {
        value = nlohmann::json::array();
    }

    int removeIndex = -1;
    int moveUpIndex = -1;
    int moveDownIndex = -1;
    int duplicateIndex = -1;

    ImGui::TextDisabled("Define the sequence of states executed by this state machine.");

    for (size_t i = 0; i < value.size(); ++i)
    {
        std::string childId = id + "[" + std::to_string(i) + "]";
        nlohmann::json &stateObj = value[i];
        if (!stateObj.is_object())
        {
            stateObj = nlohmann::json::object();
        }
        std::string currentStateName = stateObj.value("name", "");
        if (currentStateName.empty() && !m_allowedStates.empty())
        {
            currentStateName = m_allowedStates[0];
            stateObj["name"] = currentStateName;
        }

        ImGui::PushID(static_cast<int>(i));

        char headerLabel[128];
        snprintf(headerLabel, sizeof(headerLabel), "[%zu] %s##header", i, currentStateName.c_str());
        bool nodeOpen = ImGui::TreeNodeEx(headerLabel, ImGuiTreeNodeFlags_DefaultOpen);

        // Control buttons beside / on header row
        ImGui::SameLine();
        ImGui::BeginDisabled(i == 0);
        if (ImGui::SmallButton("^##up"))
            moveUpIndex = static_cast<int>(i);
        ImGui::EndDisabled();

        ImGui::SameLine();
        ImGui::BeginDisabled(i == value.size() - 1);
        if (ImGui::SmallButton("v##down"))
            moveDownIndex = static_cast<int>(i);
        ImGui::EndDisabled();

        ImGui::SameLine();
        if (ImGui::SmallButton("Copy##dup"))
            duplicateIndex = static_cast<int>(i);

        ImGui::SameLine();
        if (ImGui::SmallButton("Remove##rm"))
            removeIndex = static_cast<int>(i);

        if (nodeOpen)
        {
            // Dropdown to pick what state this can be
            ImGui::Text("State Type:");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(220);
            if (ImGui::BeginCombo("##state_type_combo", currentStateName.c_str()))
            {
                for (const auto &allowedState : m_allowedStates)
                {
                    bool isSelected = (allowedState == currentStateName);
                    if (ImGui::Selectable(allowedState.c_str(), isSelected))
                    {
                        if (allowedState != currentStateName)
                        {
                            stateObj["name"] = allowedState;
                            nlohmann::json defInputs = createDefaultInputsForState(allowedState);
                            if (defInputs.is_null())
                            {
                                stateObj.erase("inputs");
                            }
                            else
                            {
                                stateObj["inputs"] = defInputs;
                            }
                        }
                    }
                    if (isSelected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }

            // Inputs for the selected state type
            ImGui::Indent(15.0f);
            renderStateInputs(stateObj["name"].get<std::string>(), stateObj, childId, value);
            ImGui::Unindent(15.0f);

            ImGui::TreePop();
        }

        ImGui::PopID();
        ImGui::Separator();
    }

    // Apply deferred array mutations
    if (moveUpIndex > 0)
    {
        std::swap(value[moveUpIndex], value[moveUpIndex - 1]);
        m_rawBuffers.clear();
    }
    else if (moveDownIndex >= 0 && moveDownIndex + 1 < static_cast<int>(value.size()))
    {
        std::swap(value[moveDownIndex], value[moveDownIndex + 1]);
        m_rawBuffers.clear();
    }
    else if (duplicateIndex >= 0)
    {
        nlohmann::json clone = value[duplicateIndex];
        value.insert(value.begin() + duplicateIndex + 1, clone);
        m_rawBuffers.clear();
    }
    else if (removeIndex >= 0)
    {
        value.erase(value.begin() + removeIndex);
        m_rawBuffers.clear();
    }

    // Add state section
    ImGui::Spacing();
    if (!m_allowedStates.empty())
    {
        if (m_selectedNewStateType < 0 || m_selectedNewStateType >= static_cast<int>(m_allowedStates.size()))
            m_selectedNewStateType = 0;

        ImGui::SetNextItemWidth(200);
        if (ImGui::BeginCombo("##new_state_combo", m_allowedStates[m_selectedNewStateType].c_str()))
        {
            for (int s = 0; s < static_cast<int>(m_allowedStates.size()); ++s)
            {
                bool isSel = (s == m_selectedNewStateType);
                if (ImGui::Selectable(m_allowedStates[s].c_str(), isSel))
                    m_selectedNewStateType = s;
                if (isSel)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        ImGui::SameLine();
        if (ImGui::Button("+ Add State##root.states"))
        {
            const std::string &chosenState = m_allowedStates[m_selectedNewStateType];
            nlohmann::json newStateObj = nlohmann::json::object();
            newStateObj["name"] = chosenState;
            nlohmann::json defInputs = createDefaultInputsForState(chosenState);
            if (!defInputs.is_null())
                newStateObj["inputs"] = defInputs;
            value.push_back(newStateObj);
        }
    }
    else
    {
        if (ImGui::Button("Add State##root.states"))
        {
            nlohmann::json newStateObj = {{"name", "wait"}, {"inputs", {{"ms", 1000.0f}}}};
            value.push_back(newStateObj);
        }
    }
}

void DebugStateMachineEditorWindow::renderFooter()
{
    // Save buttons
    if (ImGui::Button("Save to global StateMachines"))
    {
        save_state_machine_to("/assets/DataFiles/StateMachines");
    }
    ImGui::SameLine();
    ImGui::BeginDisabled(m_levelDirectory.empty());
    if (ImGui::Button("Save to level StateMachines"))
    {
        save_state_machine_to("/" + m_levelDirectory + "/DataFiles/StateMachines");
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    if (ImGui::Button("New State Machine"))
    {
        resetToDefault();
    }

    // Load existing state machine
    if (!m_existingFiles.empty())
    {
        ImGui::Separator();
        ImGui::Text("Load Existing:");
        ImGui::SameLine();
        if (m_selectedFileToLoad < 0 || m_selectedFileToLoad >= static_cast<int>(m_existingFiles.size()))
            m_selectedFileToLoad = 0;

        ImGui::SetNextItemWidth(300);
        if (ImGui::BeginCombo("##load_sm_combo", m_existingFiles[m_selectedFileToLoad].c_str()))
        {
            for (int i = 0; i < static_cast<int>(m_existingFiles.size()); ++i)
            {
                bool isSel = (i == m_selectedFileToLoad);
                if (ImGui::Selectable(m_existingFiles[i].c_str(), isSel))
                    m_selectedFileToLoad = i;
                if (isSel)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        ImGui::SameLine();
        if (ImGui::Button("Load##file"))
        {
            loadStateMachine(m_existingFiles[m_selectedFileToLoad]);
        }
    }
}
