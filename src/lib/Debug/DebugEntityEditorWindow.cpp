#include "DebugEntityEditorWindow.h"
#include <imgui.h>

DebugEntityEditorWindow::DebugEntityEditorWindow(DataFile &dataFile)
    : DataFileBySchemaDebugWindow("Entity Editor", dataFile, DataFileSchema::EntitiesCharacter)
{
}

void DebugEntityEditorWindow::renderFooter()
{
    ImGui::Button("Save to global");
    ImGui::SameLine();
    ImGui::Button("Save to level");
}
