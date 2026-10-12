#pragma once

#include "DataFileBySchemaDebugWindow.h"

class LevelV1;

class DebugLevelEditorWindow : public DataFileBySchemaDebugWindow
{
public:
    DebugLevelEditorWindow();

    // Copies the level's entities.json data into this window's form.
    void setLevel(const LevelV1 &level);

protected:
    void renderFooter() override;

private:
    DataFile m_entities;
};
