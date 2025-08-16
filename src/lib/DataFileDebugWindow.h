#pragma once
#include "DebugWindow.h"
#include "DataFile.h"
#include <string>
#include <vector>

enum class JsonType
{
    String = 0,
    Map = 1,
    List = 2,
    Float = 3,
    Boolean = 4,
    Integer = 5
};

struct DisplayLine
{
    char key[256];
    char value[512];
    JsonType type;
    int indentLevel;
    bool isExpanded;
    std::string parentPath;

    DisplayLine(const std::string &k = "", const std::string &v = "", JsonType t = JsonType::String, int indent = 0, const std::string &parent = "")
        : type(t), indentLevel(indent), isExpanded(true), parentPath(parent)
    {
        snprintf(key, sizeof(key), "%s", k.c_str());
        snprintf(value, sizeof(value), "%s", v.c_str());
    }
};

class DataFileDebugWindow : public DebugWindow
{
public:
    DataFileDebugWindow(const std::string &title, DataFile &dataFile);
    void render() override;
    void addDisplayLine(const std::string &key, const std::string &value, JsonType type = JsonType::String, int indentLevel = 0, const std::string &parentPath = "");

private:
    DataFile &m_dataFile;
    std::vector<DisplayLine> m_displayLines;

    void updateJsonFromLines();
    void buildJsonFromLines(nlohmann::json &jsonObj, const std::string &currentPath, int currentIndent);
    void buildJsonArrayFromLines(nlohmann::json &jsonArray, const std::string &currentPath, int currentIndent);
    void populateFromJson();
    void populateFromJsonRecursive(const nlohmann::json &jsonObj, const std::string &basePath = "", int indentLevel = 0);
    std::string getJsonPath(const std::string &parentPath, const std::string &key);
    void setJsonValue(const std::string &path, const std::string &value, JsonType type);
    nlohmann::json &getJsonReference(const std::string &path);
    void removeJsonValue(const std::string &path);
    const char *getTypeString(JsonType type);
    JsonType getTypeFromJson(const nlohmann::json &value);
    void renderTypeCombo(const char *label, JsonType *type);
    std::string getValueString(const nlohmann::json &value);
};
