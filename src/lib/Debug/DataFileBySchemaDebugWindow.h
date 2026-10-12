#pragma once
#include "DebugWindow.h"
#include "DataFile.h"
#include <map>
#include <string>
#include <vector>

enum class DataFileSchema
{
    ActionsAction,
    ActionsHitbox,
    ActionsProjectiles,
    EntitiesCharacter,
    EntitiesStateMachines,
    Items,
    StateMachine,
    LevelEntities
};

// Debug window that edits a DataFile through a form generated from a JSON schema.
// Keys and types are fixed by the schema and required properties are always present.
class DataFileBySchemaDebugWindow : public DebugWindow
{
public:
    DataFileBySchemaDebugWindow(const std::string &title, DataFile &dataFile, DataFileSchema schema);
    void render() override;

    static std::string getSchemaPath(DataFileSchema schema);

protected:
    virtual void renderFooter();
    // Lets subclasses draw their own widget for the field at `id`; return true if handled.
    virtual bool renderCustomField(const std::string &id, nlohmann::json &value, const std::string &widget);
    void loadFromDataFile();
    // Copies the form's current content into `out` if it satisfies the schema.
    bool getValidWorking(nlohmann::json &out);

private:
    DataFile &m_dataFile;
    DataFileSchema m_schemaType;
    nlohmann::json m_schema;
    nlohmann::json m_working;
    std::string m_schemaError;
    std::vector<std::string> m_errors;
    std::map<std::string, std::string> m_rawBuffers;

    void loadSchema();
    void applyToDataFile();

    static nlohmann::json defaultForSchema(const nlohmann::json &schema);
    static const nlohmann::json &resolveSchema(const nlohmann::json &schema, const nlohmann::json &value);
    static std::string schemaType(const nlohmann::json &schema);
    static bool matchesType(const std::string &type, const nlohmann::json &value);
    static void normalize(nlohmann::json &value, const nlohmann::json &schema);
    static void validate(const nlohmann::json &value, const nlohmann::json &schema, const std::string &path, std::vector<std::string> &errors);

    void renderNode(const std::string &label, nlohmann::json &value, const nlohmann::json &schema, const std::string &id);
    void renderObjectBody(nlohmann::json &value, const nlohmann::json &schema, const std::string &id);
    void renderArrayBody(nlohmann::json &value, const nlohmann::json &schema, const std::string &id);
};
