#pragma once
#include "DebugWindow.h"
#include <string>
#include <vector>

// Avoid including cute/imgui headers here to prevent symbol conflicts.

class AtlasLabelerWindow : public DebugWindow
{
public:
	AtlasLabelerWindow();
	void render() override;

private:
	struct Item {
		std::string name;
		int x = 0, y = 0, w = 0, h = 0;
	};
	struct Variant {
		std::string name;
		std::string path;
	};

	// Paths
	char m_jsonPath[512];
	char m_pngPath[512];

	// Data
	int m_imageW = 0;
	int m_imageH = 0;
	std::vector<Item> m_items;
	std::vector<Variant> m_variants;
	int m_selectedVariant = 0;
	void* m_imguiTex = nullptr; // Opaque; cast to ImTextureData* in .cpp
	float m_zoom = 1.0f;
	bool m_didInitialLoad = false;

	// Helpers
	void clear();
	bool loadJSON(const std::string& path);
	bool saveJSON(const std::string& path);
	bool loadTexture(const std::string& pngPath);
	static std::string mapVFSPath(const std::string& p);
};

