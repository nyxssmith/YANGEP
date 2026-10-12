#pragma once

#include "DataFileBySchemaDebugWindow.h"

class DebugEntityEditorWindow : public DataFileBySchemaDebugWindow
{
public:
    explicit DebugEntityEditorWindow(DataFile &dataFile);

protected:
    void renderFooter() override;
};
