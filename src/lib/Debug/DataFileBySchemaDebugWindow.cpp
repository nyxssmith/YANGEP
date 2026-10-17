#include "DataFileBySchemaDebugWindow.h"
#include <imgui.h>
#include <algorithm>
#include <fstream>

namespace
{
    const nlohmann::json kEmptySchema = nlohmann::json::object();
}

std::string DataFileBySchemaDebugWindow::getSchemaPath(DataFileSchema schema)
{
    const char *file = "";
    switch (schema)
    {
    case DataFileSchema::ActionsAction:
        file = "actions_action.json";
        break;
    case DataFileSchema::ActionsHitbox:
        file = "actions_hitbox.json";
        break;
    case DataFileSchema::ActionsProjectiles:
        file = "actions_projectiles.json";
        break;
    case DataFileSchema::EntitiesCharacter:
        file = "entities_character.json";
        break;
    case DataFileSchema::EntitiesStateMachines:
        file = "entities_state_machines.json";
        break;
    case DataFileSchema::Items:
        file = "items.json";
        break;
    case DataFileSchema::StateMachine:
        file = "state_machine.json";
        break;
    case DataFileSchema::LevelEntities:
        file = "level_entities.json";
        break;
    }
    return std::string("assets/DataFiles/Schema/") + file;
}

DataFileBySchemaDebugWindow::DataFileBySchemaDebugWindow(const std::string &title, DataFile &dataFile, DataFileSchema schema)
    : DebugWindow(title), m_dataFile(dataFile), m_schemaType(schema)
{
    loadSchema();
    loadFromDataFile();
}

void DataFileBySchemaDebugWindow::loadSchema()
{
    std::string path = getSchemaPath(m_schemaType);
    std::ifstream in(path);
    if (!in)
    {
        m_schemaError = "Could not open schema: " + path;
        return;
    }
    try
    {
        in >> m_schema;
    }
    catch (const std::exception &e)
    {
        m_schemaError = "Invalid schema " + path + ": " + e.what();
    }
}

void DataFileBySchemaDebugWindow::loadFromDataFile()
{
    m_rawBuffers.clear();
    m_working = static_cast<const nlohmann::json &>(m_dataFile);
    if (m_schemaError.empty())
    {
        normalize(m_working, m_schema);
        m_errors.clear();
        validate(m_working, m_schema, "", m_errors);
    }
}

void DataFileBySchemaDebugWindow::applyToDataFile()
{
    m_errors.clear();
    validate(m_working, m_schema, "", m_errors);
    if (!m_errors.empty())
        return;
    static_cast<nlohmann::json &>(m_dataFile) = m_working;
}

bool DataFileBySchemaDebugWindow::getValidWorking(nlohmann::json &out)
{
    std::vector<std::string> errors;
    validate(m_working, m_schema, "", errors);
    if (!m_schemaError.empty() || !errors.empty())
        return false;
    out = m_working;
    return true;
}

std::string DataFileBySchemaDebugWindow::schemaType(const nlohmann::json &schema)
{
    if (schema.is_object() && schema.contains("type") && schema["type"].is_string())
    {
        std::string t = schema["type"].get<std::string>();
        if (t == "number")
            return "float";
        return t;
    }
    return "";
}

bool DataFileBySchemaDebugWindow::matchesType(const std::string &type, const nlohmann::json &value)
{
    if (type == "string")
        return value.is_string();
    if (type == "integer")
        return value.is_number_integer();
    if (type == "float")
        return value.is_number();
    if (type == "boolean")
        return value.is_boolean();
    if (type == "object")
        return value.is_object();
    if (type == "array")
        return value.is_array();
    return true;
}

// For oneOf schemas, pick the alternative matching the current value (first one otherwise).
const nlohmann::json &DataFileBySchemaDebugWindow::resolveSchema(const nlohmann::json &schema, const nlohmann::json &value)
{
    if (schema.is_object() && schema.contains("oneOf") && schema["oneOf"].is_array() && !schema["oneOf"].empty())
    {
        for (const auto &option : schema["oneOf"])
        {
            if (matchesType(schemaType(option), value))
                return option;
        }
        return schema["oneOf"][0];
    }
    return schema;
}

nlohmann::json DataFileBySchemaDebugWindow::defaultForSchema(const nlohmann::json &schemaIn)
{
    const nlohmann::json &schema = resolveSchema(schemaIn, nlohmann::json());
    std::string type = schemaType(schema);
    if (schema.contains("default"))
        return schema["default"];
    if (schema.contains("enum") && schema["enum"].is_array() && !schema["enum"].empty())
        return schema["enum"][0];
    if (type == "string")
        return "";
    if (type == "integer")
        return 0;
    if (type == "float")
        return 0.0;
    if (type == "boolean")
        return false;
    if (type == "array")
        return nlohmann::json::array();
    if (type == "object")
    {
        nlohmann::json obj = nlohmann::json::object();
        if (schema.contains("required") && schema.contains("properties"))
        {
            for (const auto &req : schema["required"])
            {
                const std::string key = req.get<std::string>();
                if (schema["properties"].contains(key))
                    obj[key] = defaultForSchema(schema["properties"][key]);
            }
        }
        return obj;
    }
    return nullptr;
}

// Coerces the value to the schema shape and fills in any missing required properties.
void DataFileBySchemaDebugWindow::normalize(nlohmann::json &value, const nlohmann::json &schemaIn)
{
    const nlohmann::json &schema = resolveSchema(schemaIn, value);
    std::string type = schemaType(schema);

    if (!matchesType(type, value))
    {
        value = defaultForSchema(schema);
        return;
    }
    if (type == "float" && value.is_number_integer())
        value = value.get<double>();

    if (type == "object" && schema.contains("properties"))
    {
        if (schema.contains("required"))
        {
            for (const auto &req : schema["required"])
            {
                const std::string key = req.get<std::string>();
                if (!value.contains(key) && schema["properties"].contains(key))
                    value[key] = defaultForSchema(schema["properties"][key]);
            }
        }
        for (auto &[key, child] : value.items())
        {
            if (schema["properties"].contains(key))
                normalize(child, schema["properties"][key]);
        }
    }
    else if (type == "array" && schema.contains("items"))
    {
        for (auto &item : value)
            normalize(item, schema["items"]);
    }
}

void DataFileBySchemaDebugWindow::validate(const nlohmann::json &value, const nlohmann::json &schemaIn, const std::string &path, std::vector<std::string> &errors)
{
    const nlohmann::json &schema = resolveSchema(schemaIn, value);
    std::string type = schemaType(schema);
    std::string where = path.empty() ? "<root>" : path;

    if (!matchesType(type, value))
    {
        errors.push_back(where + ": expected " + type);
        return;
    }
    if (schema.contains("enum") && schema["enum"].is_array())
    {
        bool found = false;
        for (const auto &option : schema["enum"])
            found = found || option == value;
        if (!found)
            errors.push_back(where + ": value not in allowed set");
    }
    if (type == "object" && schema.contains("properties"))
    {
        if (schema.contains("required"))
        {
            for (const auto &req : schema["required"])
            {
                if (!value.contains(req.get<std::string>()))
                    errors.push_back(where + ": missing required '" + req.get<std::string>() + "'");
            }
        }
        for (const auto &[key, child] : value.items())
        {
            std::string childPath = path.empty() ? key : path + "." + key;
            if (!schema["properties"].contains(key))
                errors.push_back(childPath + ": unknown property");
            else
                validate(child, schema["properties"][key], childPath, errors);
        }
    }
    else if (type == "array" && schema.contains("items"))
    {
        for (size_t i = 0; i < value.size(); ++i)
            validate(value[i], schema["items"], path + "[" + std::to_string(i) + "]", errors);
    }
}

void DataFileBySchemaDebugWindow::renderObjectBody(nlohmann::json &value, const nlohmann::json &schema, const std::string &id)
{
    if (!schema.contains("properties"))
    {
        // Free-form object: edit as raw JSON text, applied only when it parses.
        std::string &buf = m_rawBuffers[id];
        if (buf.empty())
            buf = value.dump();
        char text[1024];
        snprintf(text, sizeof(text), "%s", buf.c_str());
        ImGui::PushItemWidth(300);
        if (ImGui::InputText(("##raw" + id).c_str(), text, sizeof(text)))
        {
            buf = text;
            auto parsed = nlohmann::json::parse(buf, nullptr, false);
            if (parsed.is_object())
                value = parsed;
        }
        ImGui::PopItemWidth();
        return;
    }

    std::vector<std::string> required;
    if (schema.contains("required"))
        for (const auto &r : schema["required"])
            required.push_back(r.get<std::string>());

    for (const auto &[key, propSchema] : schema["properties"].items())
    {
        bool isRequired = std::find(required.begin(), required.end(), key) != required.end();
        std::string childId = id + "." + key;
        std::string label = key + (isRequired ? " *" : "");

        if (!value.contains(key))
        {
            ImGui::TextDisabled("%s", label.c_str());
            ImGui::SameLine();
            if (ImGui::Button(("Add##" + childId).c_str()))
                value[key] = defaultForSchema(propSchema);
            continue;
        }

        renderNode(label, value[key], propSchema, childId);
        if (!isRequired)
        {
            ImGui::SameLine();
            if (ImGui::SmallButton(("Remove##" + childId).c_str()))
            {
                value.erase(key);
                m_rawBuffers.clear();
            }
        }
    }
}

void DataFileBySchemaDebugWindow::renderArrayBody(nlohmann::json &value, const nlohmann::json &schema, const std::string &id)
{
    const nlohmann::json &itemSchema = schema.contains("items") ? schema["items"] : kEmptySchema;
    int removeIndex = -1;
    for (size_t i = 0; i < value.size(); ++i)
    {
        std::string childId = id + "[" + std::to_string(i) + "]";
        renderNode("[" + std::to_string(i) + "]", value[i], itemSchema, childId);
        ImGui::SameLine();
        if (ImGui::SmallButton(("Remove##" + childId).c_str()))
            removeIndex = static_cast<int>(i);
    }
    if (removeIndex >= 0)
    {
        value.erase(value.begin() + removeIndex);
        m_rawBuffers.clear();
    }
    if (ImGui::Button(("Add Item##" + id).c_str()))
        value.push_back(defaultForSchema(itemSchema));
}

void DataFileBySchemaDebugWindow::renderNode(const std::string &label, nlohmann::json &value, const nlohmann::json &schemaIn, const std::string &id)
{
    const nlohmann::json &schema = resolveSchema(schemaIn, value);
    std::string type = schemaType(schema);
    std::string widget = "##" + id;

    if (type == "object" || type == "array")
    {
        bool open = ImGui::TreeNodeEx((label + widget).c_str(), ImGuiTreeNodeFlags_DefaultOpen);
        if (open)
        {
            if (schema.contains("description") && schema["description"].is_string())
                ImGui::TextDisabled("%s", schema["description"].get<std::string>().c_str());
            if (type == "object" && value.is_object())
                renderObjectBody(value, schema, id);
            else if (type == "array" && value.is_array())
                renderArrayBody(value, schema, id);
            ImGui::TreePop();
        }
        return;
    }

    // Description goes above the row so a trailing Remove button stays beside the field.
    if (schema.contains("description") && schema["description"].is_string())
        ImGui::TextDisabled("%s", schema["description"].get<std::string>().c_str());

    // Fixed-width label column measured from the current indent so nested fields don't overlap.
    const float labelColumnWidth = 260.0f;
    float labelStartX = ImGui::GetCursorPosX();
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted(label.c_str());
    ImGui::SameLine(labelStartX + labelColumnWidth);
    ImGui::PushItemWidth(200);

    if (schema.contains("enum") && schema["enum"].is_array() && !schema["enum"].empty())
    {
        std::string current = value.is_string() ? value.get<std::string>() : value.dump();
        if (ImGui::BeginCombo(widget.c_str(), current.c_str()))
        {
            for (const auto &option : schema["enum"])
            {
                std::string name = option.is_string() ? option.get<std::string>() : option.dump();
                if (ImGui::Selectable(name.c_str(), option == value))
                    value = option;
            }
            ImGui::EndCombo();
        }
    }
    else if (renderCustomField(id, value, widget))
    {
    }
    else if (type == "string" && value.is_string())
    {
        char buf[512];
        snprintf(buf, sizeof(buf), "%s", value.get<std::string>().c_str());
        if (ImGui::InputText(widget.c_str(), buf, sizeof(buf)))
            value = std::string(buf);
    }
    else if (type == "integer" && value.is_number_integer())
    {
        long long v = value.get<long long>();
        if (ImGui::InputScalar(widget.c_str(), ImGuiDataType_S64, &v))
            value = v;
    }
    else if (type == "float" && value.is_number())
    {
        double v = value.get<double>();
        if (ImGui::InputDouble(widget.c_str(), &v, 0.0, 0.0, "%.4f"))
            value = v;
    }
    else if (type == "boolean" && value.is_boolean())
    {
        bool v = value.get<bool>();
        if (ImGui::Checkbox(widget.c_str(), &v))
            value = v;
    }
    else
    {
        ImGui::TextDisabled("%s", value.dump().c_str());
    }
    ImGui::PopItemWidth();
}

void DataFileBySchemaDebugWindow::render()
{
    if (!m_show)
        return;

    ImGui::Begin(m_title.c_str(), &m_show);

    const std::string &path = m_dataFile.getpath();
    ImGui::Text("File: %s", path.empty() ? "<no file loaded>" : path.c_str());
    ImGui::Text("Schema: %s", getSchemaPath(m_schemaType).c_str());
    ImGui::Separator();

    if (!m_schemaError.empty())
    {
        ImGui::TextColored(ImVec4(1, 0.3f, 0.3f, 1), "%s", m_schemaError.c_str());
        ImGui::End();
        return;
    }

    ImGui::TextDisabled("* = required");
    renderNode("root", m_working, m_schema, "root");

    m_errors.clear();
    validate(m_working, m_schema, "", m_errors);

    ImGui::Separator();
    if (m_errors.empty())
    {
        ImGui::TextColored(ImVec4(0.3f, 1, 0.3f, 1), "Valid");
    }
    else
    {
        ImGui::TextColored(ImVec4(1, 0.3f, 0.3f, 1), "%zu schema error(s):", m_errors.size());
        for (const auto &err : m_errors)
            ImGui::BulletText("%s", err.c_str());
    }

    ImGui::Separator();
    ImGui::BeginDisabled(!m_errors.empty());
    if (ImGui::Button("Apply Window -> JSON"))
        applyToDataFile();
    ImGui::SameLine();
    if (ImGui::Button("Apply & Save File"))
    {
        applyToDataFile();
        if (!m_dataFile.getpath().empty())
            m_dataFile.save();
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    if (ImGui::Button("Reload JSON -> Window"))
        loadFromDataFile();

    ImGui::Separator();
    renderFooter();

    ImGui::End();
}

void DataFileBySchemaDebugWindow::renderFooter()
{
}

bool DataFileBySchemaDebugWindow::renderCustomField(const std::string &, nlohmann::json &, const std::string &)
{
    return false;
}
