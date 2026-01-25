#include "HudUI.h"
#include "../Items/Inventory.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <nlohmann/json.hpp>
#include <spng.h>
#include <unordered_map>
#include <fstream>
#include <cstdio>
#include <cstring>

struct HudAtlasRect {
	int x = 0;
	int y = 0;
	int w = 0;
	int h = 0;
};

struct HudUIAssets {
	bool loaded = false;
	ImTextureData* atlas_tex = nullptr;
	int atlas_w = 0;
	int atlas_h = 0;
	std::unordered_map<std::string, HudAtlasRect> atlas_items;

	ImTextureData* paper_tex = nullptr;
	int paper_w = 0;
	int paper_h = 0;
};

static std::string s_map_vfs_path(const std::string& path)
{
	if (path.rfind("/assets/", 0) == 0) {
		const char* vfs_prefix = "/assets/";
		size_t prefix_len = strlen(vfs_prefix); // 8
		return std::string("assets/") + path.substr(prefix_len);
	}
	return path;
}

static bool s_load_png_rgba8(const std::string& path, std::vector<unsigned char>& out, int& w, int& h)
{
	FILE* f = fopen(path.c_str(), "rb");
	if (!f) return false;

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

static ImTextureData* s_create_imgui_texture(const std::vector<unsigned char>& pixels, int w, int h)
{
	ImTextureData* tex = IM_NEW(ImTextureData)();
	tex->Create(ImTextureFormat_RGBA32, w, h);
	unsigned char* dst = (unsigned char*)tex->GetPixels();
	memcpy(dst, pixels.data(), (size_t)w * h * 4);
	ImGui::RegisterUserTexture(tex);
	return tex;
}

static HudUIAssets& s_assets()
{
	static HudUIAssets a;
	return a;
}

static void s_load_assets_once()
{
	HudUIAssets& a = s_assets();
	if (a.loaded) return;
	a.loaded = true;

	// Load atlas JSON + atlas PNG
	const std::string json_path = s_map_vfs_path("/assets/GUI/RPG_GUI_moonmod_source_autocut.json");
	std::ifstream ifs(json_path);
	if (ifs) {
		nlohmann::json j;
		try {
			ifs >> j;
			if (j.contains("image_width")) a.atlas_w = j["image_width"].get<int>();
			if (j.contains("image_height")) a.atlas_h = j["image_height"].get<int>();
			if (j.contains("items") && j["items"].is_array()) {
				for (const auto& it : j["items"]) {
					if (!it.contains("name") || !it.contains("rect")) continue;
					const std::string name = it["name"].get<std::string>();
					const auto& r = it["rect"];
					if (!r.is_array() || r.size() < 4) continue;
					HudAtlasRect rect;
					rect.x = r[0].get<int>();
					rect.y = r[1].get<int>();
					rect.w = r[2].get<int>();
					rect.h = r[3].get<int>();
					a.atlas_items[name] = rect;
				}
			}
			if (j.contains("atlas") && j["atlas"].is_string()) {
				std::string atlas_path = s_map_vfs_path(j["atlas"].get<std::string>());
				std::vector<unsigned char> pixels;
				int w = 0, h = 0;
				if (s_load_png_rgba8(atlas_path, pixels, w, h)) {
					a.atlas_w = w;
					a.atlas_h = h;
					a.atlas_tex = s_create_imgui_texture(pixels, w, h);
				}
			}
		} catch (...) {
			// Ignore; assets will remain null.
		}
	}

	// Load paper background
	const std::string paper_path = s_map_vfs_path("/assets/GUI/paper_background.png");
	std::vector<unsigned char> paper_pixels;
	int pw = 0, ph = 0;
	if (s_load_png_rgba8(paper_path, paper_pixels, pw, ph)) {
		a.paper_w = pw;
		a.paper_h = ph;
		a.paper_tex = s_create_imgui_texture(paper_pixels, pw, ph);
	}
}

static bool s_get_rect(const char* name, HudAtlasRect& out)
{
	HudUIAssets& a = s_assets();
	auto it = a.atlas_items.find(name);
	if (it == a.atlas_items.end()) return false;
	out = it->second;
	return true;
}

static void s_uv_from_rect(const HudAtlasRect& r, int w, int h, ImVec2& uv0, ImVec2& uv1)
{
	uv0 = ImVec2((float)r.x / (float)w, (float)r.y / (float)h);
	uv1 = ImVec2((float)(r.x + r.w) / (float)w, (float)(r.y + r.h) / (float)h);
}

void HudUI::initialize()
{
	s_load_assets_once();
}

void HudUI::renderLeftColumn(const std::vector<Icon>& icons, float iconSize, float padding)
{
	s_load_assets_once();
	const float margin = 12.0f;
	const int count = (int)icons.size();
	if (count == 0) return;

	HudAtlasRect rect;
	const bool has_rect = s_get_rect("icon_background_large", rect);
	const float size = (iconSize > 0.0f) ? iconSize : (has_rect ? (float)rect.w : 48.0f);
	const float total_h = count * size + (count - 1) * padding;

	ImGui::SetNextWindowPos(ImVec2(margin, margin));
	ImGui::SetNextWindowSize(ImVec2(size + 8.0f, total_h + 8.0f));
	ImGui::SetNextWindowBgAlpha(0.0f);
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
	                         ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
	                         ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoDecoration;
	ImGui::Begin("HUD_LeftColumn", nullptr, flags);

	if (has_rect && s_assets().atlas_tex) {
		ImVec2 uv0, uv1;
		s_uv_from_rect(rect, s_assets().atlas_w, s_assets().atlas_h, uv0, uv1);
		ImTextureRef texref = s_assets().atlas_tex->GetTexRef();
		for (int i = 0; i < count; ++i) {
			ImGui::PushID(i);
			ImGui::ImageButton("icon", texref, ImVec2(size, size), uv0, uv1);
			if (i < count - 1) ImGui::Dummy(ImVec2(0.0f, padding));
			ImGui::PopID();
		}
	} else {
		for (int i = 0; i < count; ++i) {
			ImGui::PushID(i);
			ImGui::Button("icon", ImVec2(size, size));
			if (i < count - 1) ImGui::Dummy(ImVec2(0.0f, padding));
			ImGui::PopID();
		}
	}

	ImGui::End();
}

void HudUI::renderBottomRow(const std::vector<Icon>& icons, float iconSize, float padding)
{
	s_load_assets_once();
	const float margin = 12.0f;
	const int count = (int)icons.size();
	if (count == 0) return;

	HudAtlasRect rect;
	const bool has_rect = s_get_rect("icon_background_large", rect);
	const float size = (iconSize > 0.0f) ? iconSize : (has_rect ? (float)rect.w : 48.0f);
	const float total_w = count * size + (count - 1) * padding;
	ImVec2 display = ImGui::GetIO().DisplaySize;

	ImGui::SetNextWindowPos(ImVec2((display.x - total_w) * 0.5f, display.y - size - margin));
	ImGui::SetNextWindowSize(ImVec2(total_w + 8.0f, size + 8.0f));
	ImGui::SetNextWindowBgAlpha(0.0f);
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
	                         ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
	                         ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoDecoration;
	ImGui::Begin("HUD_BottomRow", nullptr, flags);

	if (has_rect && s_assets().atlas_tex) {
		ImVec2 uv0, uv1;
		s_uv_from_rect(rect, s_assets().atlas_w, s_assets().atlas_h, uv0, uv1);
		ImTextureRef texref = s_assets().atlas_tex->GetTexRef();
		for (int i = 0; i < count; ++i) {
			ImGui::PushID(i);
			ImGui::ImageButton("icon", texref, ImVec2(size, size), uv0, uv1);
			if (i < count - 1) ImGui::SameLine(0.0f, padding);
			ImGui::PopID();
		}
	} else {
		for (int i = 0; i < count; ++i) {
			ImGui::PushID(i);
			ImGui::Button("icon", ImVec2(size, size));
			if (i < count - 1) ImGui::SameLine(0.0f, padding);
			ImGui::PopID();
		}
	}

	ImGui::End();
}

void HudUI::renderInventoryWindow(const Inventory* inventory, bool* open, int cols, float slotSize, float slotPad)
{
	s_load_assets_once();
	if (!open || !*open) return;

	ImVec2 display = ImGui::GetIO().DisplaySize;
	ImVec2 window_size(520.0f, 380.0f);
	ImGui::SetNextWindowPos(ImVec2((display.x - window_size.x) * 0.5f, (display.y - window_size.y) * 0.5f), ImGuiCond_Always);
	ImGui::SetNextWindowSize(window_size, ImGuiCond_Always);
	ImGui::SetNextWindowBgAlpha(0.0f);
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
	                         ImGuiWindowFlags_NoSavedSettings;
	ImGui::Begin("Inventory", open, flags);

	// Background texture (paper)
	if (s_assets().paper_tex) {
		ImDrawList* dl = ImGui::GetWindowDrawList();
		ImVec2 win_pos = ImGui::GetWindowPos();
		ImVec2 win_size = ImGui::GetWindowSize();
		ImTextureRef paper_ref = s_assets().paper_tex->GetTexRef();
		dl->AddImage(paper_ref, win_pos, ImVec2(win_pos.x + win_size.x, win_pos.y + win_size.y));
	}

	// Border using atlas edge/corner pieces (stretched)
	HudAtlasRect c_ul, c_ur, c_ll, c_lr, e_h, e_v;
	const bool has_border =
		s_get_rect("border_double_corner_upper_left", c_ul) &&
		s_get_rect("border_double_corner_upper_right", c_ur) &&
		s_get_rect("border_double_corner_lower_left", c_ll) &&
		s_get_rect("border_double_corner_lower_right", c_lr) &&
		s_get_rect("border_double_large_edge_horizontal", e_h) &&
		s_get_rect("border_double_large_edge_vertical", e_v) &&
		s_assets().atlas_tex;

	if (has_border) {
		ImDrawList* dl = ImGui::GetWindowDrawList();
		ImVec2 win_pos = ImGui::GetWindowPos();
		ImVec2 win_size = ImGui::GetWindowSize();
		ImTextureRef atlas_ref = s_assets().atlas_tex->GetTexRef();

		ImVec2 uv0, uv1;
		// Corners
		s_uv_from_rect(c_ul, s_assets().atlas_w, s_assets().atlas_h, uv0, uv1);
		dl->AddImage(atlas_ref, win_pos, ImVec2(win_pos.x + c_ul.w, win_pos.y + c_ul.h), uv0, uv1);
		s_uv_from_rect(c_ur, s_assets().atlas_w, s_assets().atlas_h, uv0, uv1);
		dl->AddImage(atlas_ref, ImVec2(win_pos.x + win_size.x - c_ur.w, win_pos.y),
		             ImVec2(win_pos.x + win_size.x, win_pos.y + c_ur.h), uv0, uv1);
		s_uv_from_rect(c_ll, s_assets().atlas_w, s_assets().atlas_h, uv0, uv1);
		dl->AddImage(atlas_ref, ImVec2(win_pos.x, win_pos.y + win_size.y - c_ll.h),
		             ImVec2(win_pos.x + c_ll.w, win_pos.y + win_size.y), uv0, uv1);
		s_uv_from_rect(c_lr, s_assets().atlas_w, s_assets().atlas_h, uv0, uv1);
		dl->AddImage(atlas_ref, ImVec2(win_pos.x + win_size.x - c_lr.w, win_pos.y + win_size.y - c_lr.h),
		             ImVec2(win_pos.x + win_size.x, win_pos.y + win_size.y), uv0, uv1);

		// Edges (stretched)
		s_uv_from_rect(e_h, s_assets().atlas_w, s_assets().atlas_h, uv0, uv1);
		dl->AddImage(atlas_ref, ImVec2(win_pos.x + c_ul.w, win_pos.y),
		             ImVec2(win_pos.x + win_size.x - c_ur.w, win_pos.y + e_h.h), uv0, uv1);
		dl->AddImage(atlas_ref, ImVec2(win_pos.x + c_ll.w, win_pos.y + win_size.y - e_h.h),
		             ImVec2(win_pos.x + win_size.x - c_lr.w, win_pos.y + win_size.y), uv0, uv1);
		s_uv_from_rect(e_v, s_assets().atlas_w, s_assets().atlas_h, uv0, uv1);
		dl->AddImage(atlas_ref, ImVec2(win_pos.x, win_pos.y + c_ul.h),
		             ImVec2(win_pos.x + e_v.w, win_pos.y + win_size.y - c_ll.h), uv0, uv1);
		dl->AddImage(atlas_ref, ImVec2(win_pos.x + win_size.x - e_v.w, win_pos.y + c_ur.h),
		             ImVec2(win_pos.x + win_size.x, win_pos.y + win_size.y - c_lr.h), uv0, uv1);
	}

	ImGui::Text("Inventory");
	ImGui::Separator();

	const int total = inventory ? (int)inventory->getCapacity() : 0;
	const int rows = (cols > 0) ? (int)((total + cols - 1) / cols) : 0;

	HudAtlasRect slot_rect;
	const bool has_slot = s_get_rect("icon_background_large", slot_rect) && s_assets().atlas_tex;
	ImTextureRef atlas_ref = has_slot ? s_assets().atlas_tex->GetTexRef() : ImTextureRef();
	ImVec2 slot_uv0, slot_uv1;
	if (has_slot) s_uv_from_rect(slot_rect, s_assets().atlas_w, s_assets().atlas_h, slot_uv0, slot_uv1);

	for (int r = 0; r < rows; ++r) {
		for (int c = 0; c < cols; ++c) {
			const int idx = r * cols + c;
			if (idx >= total) break;

			ImGui::PushID(idx);
			if (has_slot) {
				ImGui::ImageButton("slot", atlas_ref, ImVec2(slotSize, slotSize), slot_uv0, slot_uv1);
			} else {
				ImGui::Button("slot", ImVec2(slotSize, slotSize));
			}

			if (inventory) {
				auto item = inventory->getItem((size_t)idx);
				if (item.has_value()) {
					ImVec2 p = ImGui::GetItemRectMin();
					ImDrawList* dl = ImGui::GetWindowDrawList();
					const std::string label = item->name();
					dl->AddText(ImVec2(p.x + 6.0f, p.y + 6.0f), IM_COL32(255, 255, 255, 230), label.c_str());
				}
			}

			ImGui::PopID();
			if (c < cols - 1) ImGui::SameLine(0.0f, slotPad);
		}
	}

	ImGui::End();
}
