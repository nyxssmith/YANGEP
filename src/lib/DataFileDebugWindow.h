#pragma once
#include "DebugWindow.h"
#include "DataFile.h"
#include <string>
#include <vector>

struct DisplayLine
{
    char boxA[256];
    char boxB[256];

    DisplayLine(const std::string &a = "Box A", const std::string &b = "Box B")
    {
        snprintf(boxA, sizeof(boxA), "%s", a.c_str());
        snprintf(boxB, sizeof(boxB), "%s", b.c_str());
    }
};

class DataFileDebugWindow : public DebugWindow
{
public:
    DataFileDebugWindow(const std::string &title, DataFile &dataFile);
    void render() override;
    void addDisplayLine(const std::string &boxA, const std::string &boxB);

private:
    DataFile &m_dataFile;
    std::vector<DisplayLine> m_displayLines;

    void updateJsonText();
    void populateFromJson();
};
