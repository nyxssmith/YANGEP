#include "DebugLevelEditorWindow.h"
#include "LevelV1.h"
#include <imgui.h>

// m_entities is only bound by reference here; the base never reads it until setLevel() runs.
DebugLevelEditorWindow::DebugLevelEditorWindow()
    : DataFileBySchemaDebugWindow("Level Editor", m_entities, DataFileSchema::LevelEntities)
{
}

void DebugLevelEditorWindow::setLevel(const LevelV1 &level)
{
    m_entities = level.getEntities();
    loadFromDataFile();
}

void DebugLevelEditorWindow::renderFooter()
{
    ImGui::Button("Add entity");
}
