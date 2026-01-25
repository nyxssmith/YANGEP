#include "AtlasLabelerWindow.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <cstdio>
#include <cstring>
#include <spng.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <cute.h>

static std::string s_map_vfs_path(const std::string& path)
{
	if (path.rfind("/assets/", 0) == 0) {
		const char* vfs_prefix = "/assets/";
		size_t prefix_len = strlen(vfs_prefix); // 8
		return std::string("assets/") + path.substr(prefix_len);
	}
	return path;
}

AtlasLabelerWindow::AtlasLabelerWindow()
	: DebugWindow("Atlas Labeler")
{
	snprintf(m_jsonPath, sizeof(m_jsonPath), "/assets/GUI/RPG_GUI_moonmod_source_autocut.json");
	snprintf(m_pngPath, sizeof(m_pngPath), "/assets/GUI/RPG_GUI_moonmod_source.png");
	m_zoom = 0.75f;
}

void AtlasLabelerWindow::clear()
{
	m_items.clear();
	m_variants.clear();
	m_selectedVariant = 0;
	m_imageW = 0;
	m_imageH = 0;
	if (m_imguiTex) {
		ImTextureData* tex = (ImTextureData*)m_imguiTex;
		delete tex;
		m_imguiTex = nullptr;
	}
}

bool AtlasLabelerWindow::loadJSON(const std::string& path)
{
	clear();
	const std::string mapped = s_map_vfs_path(path);
	const bool vfs_exists = Cute::fs_file_exists(path.c_str());
	const char* vfs_actual = Cute::fs_get_actual_path(path.c_str());
	printf("AtlasLabeler: Opening JSON mapped path: %s (from %s)\n", mapped.c_str(), path.c_str());
	printf("AtlasLabeler: VFS exists=%d actual=%s\n", vfs_exists ? 1 : 0, vfs_actual ? vfs_actual : "<null>");
	fflush(stdout);
	std::ifstream ifs(mapped);
	if (!ifs) {
		printf("AtlasLabeler: Failed to open JSON: %s\n", path.c_str());
		fflush(stdout);
		return false;
	}
	std::string json_text((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
	nlohmann::json j;
	try {
		j = nlohmann::json::parse(json_text);
	} catch (...) {
		printf("AtlasLabeler: Failed to parse JSON: %s\n", path.c_str());
		printf("AtlasLabeler: JSON size=%zu bytes\n", json_text.size());
		const size_t dump_len = std::min<size_t>(json_text.size(), 2048);
		printf("AtlasLabeler: JSON head (%zu bytes):\n%.*s\n", dump_len, (int)dump_len, json_text.c_str());
		fflush(stdout);
		return false;
	}
	if (j.contains("image_width")) m_imageW = j["image_width"].get<int>();
	if (j.contains("image_height")) m_imageH = j["image_height"].get<int>();
	// Back-compat: single atlas path
	if (j.contains("atlas") && j["atlas"].is_string()) {
		std::string atlasPath = j["atlas"];
		snprintf(m_pngPath, sizeof(m_pngPath), "%s", atlasPath.c_str());
		Variant v;
		v.name = "base";
		v.path = atlasPath;
		m_variants.push_back(v);
	}
	// Variants object: { name: path }
	if (j.contains("variants") && j["variants"].is_object()) {
		m_variants.clear();
		for (auto it = j["variants"].begin(); it != j["variants"].end(); ++it) {
			Variant v;
			v.name = it.key();
			v.path = it.value().get<std::string>();
			m_variants.push_back(v);
		}
		if (!m_variants.empty()) {
			snprintf(m_pngPath, sizeof(m_pngPath), "%s", m_variants[0].path.c_str());
		}
	}
	if (m_variants.empty()) {
		Variant v; v.name = "base"; v.path = m_pngPath;
		m_variants.push_back(v);
	}

	if (j.contains("items") && j["items"].is_array()) {
		for (auto& it : j["items"]) {
			Item item;
			item.name = it.contains("name") && it["name"].is_string() ? it["name"].get<std::string>() : "";
			if (it.contains("rect") && it["rect"].is_array() && it["rect"].size() == 4) {
				item.x = it["rect"][0].get<int>();
				item.y = it["rect"][1].get<int>();
				item.w = it["rect"][2].get<int>();
				item.h = it["rect"][3].get<int>();
			}
			m_items.push_back(item);
		}
	}
	// Attempt to load PNG texture after JSON (path available)
	loadTexture(m_pngPath);
	printf("AtlasLabeler: Loaded %zu items from %s\n", m_items.size(), path.c_str());
	fflush(stdout);
	return true;
}

bool AtlasLabelerWindow::saveJSON(const std::string& path)
{
	nlohmann::json j;
	// Keep atlas for back-compat (use first variant)
	j["atlas"] = (!m_variants.empty() ? m_variants[0].path : m_pngPath);
	// Write variants dictionary
	nlohmann::json vobj = nlohmann::json::object();
	for (const auto& v : m_variants) {
		vobj[v.name] = v.path;
	}
	j["variants"] = vobj;
	j["image_width"] = m_imageW;
	j["image_height"] = m_imageH;
	j["items"] = nlohmann::json::array();
	for (const auto& it : m_items) {
		nlohmann::json e;
		e["name"] = it.name;
		e["rect"] = { it.x, it.y, it.w, it.h };
		j["items"].push_back(e);
	}
	std::ofstream ofs(s_map_vfs_path(path));
	if (!ofs) {
		printf("AtlasLabeler: Failed to write JSON: %s\n", path.c_str());
		return false;
	}
	ofs << j.dump(2) << "\n";
	printf("AtlasLabeler: Saved %zu items to %s\n", m_items.size(), path.c_str());
	fflush(stdout);
	return true;
}

static bool load_png_rgba8_runtime(const char* path, std::vector<unsigned char>& out, int& w, int& h)
{
	// Map CF VFS path "/assets/..." to real relative path "assets/..." when using stdio.
	char mapped[1024];
	snprintf(mapped, sizeof(mapped), "%s", path);
	const char* vfs_prefix = "/assets/";
	if (strncmp(mapped, vfs_prefix, strlen(vfs_prefix)) == 0) {
		// Rewrite to relative path under CWD (build dir has "assets/...")
		snprintf(mapped, sizeof(mapped), "assets/%s", path + (int)strlen(vfs_prefix));
	}

	printf("AtlasLabeler: Opening PNG mapped path: %s (from %s)\n", mapped, path);
	fflush(stdout);
	FILE* f = fopen(mapped, "rb");
	if (!f) {
		// Fallback: try original path as-is (absolute or other)
		printf("AtlasLabeler: Fallback open original PNG path: %s\n", path);
		fflush(stdout);
		f = fopen(path, "rb");
	}
	if (!f) {
		printf("AtlasLabeler: Failed to open PNG: %s\n", path);
		fflush(stdout);
		return false;
	}
	spng_ctx* ctx = spng_ctx_new(0);
	if (!ctx) { fclose(f); return false; }
	spng_set_png_file(ctx, f);
	spng_ihdr ihdr{};
	if (spng_get_ihdr(ctx, &ihdr)) { spng_ctx_free(ctx); fclose(f); return false; }
	w = (int)ihdr.width;
	h = (int)ihdr.height;
	size_t sz = 0;
	if (spng_decoded_image_size(ctx, SPNG_FMT_RGBA8, &sz)) { spng_ctx_free(ctx); fclose(f); return false; }
	out.resize(sz);
	if (spng_decode_image(ctx, out.data(), sz, SPNG_FMT_RGBA8, 0)) { spng_ctx_free(ctx); fclose(f); return false; }
	spng_ctx_free(ctx);
	fclose(f);
	return true;
}

bool AtlasLabelerWindow::loadTexture(const std::string& pngPath)
{
	std::vector<unsigned char> pixels;
	int w = 0, h = 0;
	if (!load_png_rgba8_runtime(pngPath.c_str(), pixels, w, h)) return false;
	m_imageW = w;
	m_imageH = h;

	if (m_imguiTex) {
		ImTextureData* old = (ImTextureData*)m_imguiTex;
		ImGui::UnregisterUserTexture(old);
		delete old;
		m_imguiTex = nullptr;
	}
	ImTextureData* tex = IM_NEW(ImTextureData)();
	tex->Create(ImTextureFormat_RGBA32, w, h);
	unsigned char* dst = (unsigned char*)tex->GetPixels();
	memcpy(dst, pixels.data(), (size_t)w * h * 4);
	ImGui::RegisterUserTexture(tex);
	m_imguiTex = tex;
	return true;
}

void AtlasLabelerWindow::render()
{
	if (!m_show) return;
	ImGui::Begin(m_title.c_str(), &m_show);

	// Auto-load once on first open to populate from defaults.
	if (!m_didInitialLoad) {
		m_didInitialLoad = true;
		if (!loadJSON(m_jsonPath)) {
			loadTexture(m_pngPath);
		}
	}

	ImGui::Text("Atlas JSON");
	ImGui::InputText("##json", m_jsonPath, sizeof(m_jsonPath));
	if (ImGui::Button("Load")) {
		loadJSON(m_jsonPath);
	}
	ImGui::SameLine();
	if (ImGui::Button("Save")) {
		saveJSON(m_jsonPath);
	}

	ImGui::Separator();
	ImGui::Text("Atlas PNG");
	ImGui::InputText("##png", m_pngPath, sizeof(m_pngPath));
	ImGui::Text("Image size: %d x %d", m_imageW, m_imageH);
	if (ImGui::Button("Load PNG")) {
		loadTexture(m_pngPath);
	}
	ImGui::SliderFloat("Zoom", &m_zoom, 0.1f, 4.0f, "%.2f");

	ImGui::Separator();
	ImGui::Text("Variants");
	// Variant selector
	if (!m_variants.empty()) {
		// Build preview label
		if (m_selectedVariant < 0 || m_selectedVariant >= (int)m_variants.size()) m_selectedVariant = 0;
		const char* current = m_variants[m_selectedVariant].name.c_str();
		if (ImGui::BeginCombo("##variant", current)) {
			for (int i = 0; i < (int)m_variants.size(); ++i) {
				bool sel = (i == m_selectedVariant);
				if (ImGui::Selectable(m_variants[i].name.c_str(), sel)) {
					m_selectedVariant = i;
					snprintf(m_pngPath, sizeof(m_pngPath), "%s", m_variants[i].path.c_str());
					loadTexture(m_pngPath);
				}
			}
			ImGui::EndCombo();
		}
		// Path edit for selected variant
		ImGui::InputText("Path", m_pngPath, sizeof(m_pngPath));
		if (ImGui::Button("Reload Variant")) {
			if (m_selectedVariant >= 0 && m_selectedVariant < (int)m_variants.size()) {
				m_variants[m_selectedVariant].path = m_pngPath;
				loadTexture(m_pngPath);
			}
		}
	}
	ImGui::Separator();
	ImGui::Text("Items (%d)", (int)m_items.size());

	ImGui::BeginChild("items_list", ImVec2(0, 300), true);
	for (size_t i = 0; i < m_items.size(); ++i) {
		ImGui::PushID((int)i);
		auto& it = m_items[i];
		ImGui::Text("[%03d]", (int)i);
		ImGui::SameLine();
		static char nameBuf[256];
		snprintf(nameBuf, sizeof(nameBuf), "%s", it.name.c_str());
		if (ImGui::InputText("##name", nameBuf, sizeof(nameBuf))) {
			it.name = nameBuf;
		}
		ImGui::SameLine();
		ImGui::Text(" rect: (%d,%d,%d,%d)", it.x, it.y, it.w, it.h);
		ImGui::PopID();
	}
	ImGui::EndChild();

	ImGui::Separator();
	if (m_imguiTex) {
		// Scrollable preview with overlays
		ImGui::BeginChild("preview", ImVec2(0, 0), true);
		ImVec2 size = ImVec2((float)m_imageW * m_zoom, (float)m_imageH * m_zoom);
		ImTextureData* tex = (ImTextureData*)m_imguiTex;
		ImGui::Image(tex->GetTexRef(), size);
		ImVec2 min = ImGui::GetItemRectMin();
		ImVec2 max = ImGui::GetItemRectMax();
		ImDrawList* dl = ImGui::GetWindowDrawList();
		const ImU32 col = IM_COL32(0, 255, 0, 200);
		for (size_t i = 0; i < m_items.size(); ++i) {
			const auto& it = m_items[i];
			ImVec2 a = ImVec2(min.x + it.x * m_zoom, min.y + it.y * m_zoom);
			ImVec2 b = ImVec2(a.x + it.w * m_zoom, a.y + it.h * m_zoom);
			dl->AddRect(a, b, col);
			char label[32];
			snprintf(label, sizeof(label), "%zu", i);
			ImVec2 text_pos = ImVec2(a.x + 2.0f, a.y + 2.0f);
			ImVec2 text_size = ImGui::CalcTextSize(label);
			ImU32 bg = IM_COL32(0, 0, 0, 160);
			dl->AddRectFilled(ImVec2(text_pos.x - 2.0f, text_pos.y - 1.0f),
			                  ImVec2(text_pos.x + text_size.x + 2.0f, text_pos.y + text_size.y + 1.0f),
			                  bg);
			dl->AddText(text_pos, IM_COL32(255, 255, 255, 230), label);
		}
		ImGui::EndChild();
	} else {
		ImGui::Text("No texture loaded.");
	}

	ImGui::End();
}

