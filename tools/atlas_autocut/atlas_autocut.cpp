#include <spng.h>
#include <nlohmann/json.hpp>
#include <vector>
#include <queue>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <algorithm>

struct Rect { int x, y, w, h; };

static bool load_png_rgba8(const std::string& path, std::vector<uint8_t>& out, int& w, int& h)
{
	FILE* f = fopen(path.c_str(), "rb");
	if (!f) {
		std::cerr << "atlas_autocut: Failed to open " << path << "\n";
		return false;
	}

	spng_ctx* ctx = spng_ctx_new(0);
	if (!ctx) {
		fclose(f);
		std::cerr << "atlas_autocut: Failed to create spng context\n";
		return false;
	}
	spng_set_png_file(ctx, f);

	spng_ihdr ihdr{};
	int ret = spng_get_ihdr(ctx, &ihdr);
	if (ret) {
		std::cerr << "atlas_autocut: spng_get_ihdr error " << ret << "\n";
		spng_ctx_free(ctx);
		fclose(f);
		return false;
	}
	w = (int)ihdr.width;
	h = (int)ihdr.height;

	size_t image_size = 0;
	ret = spng_decoded_image_size(ctx, SPNG_FMT_RGBA8, &image_size);
	if (ret) {
		std::cerr << "atlas_autocut: decoded_image_size error " << ret << "\n";
		spng_ctx_free(ctx);
		fclose(f);
		return false;
	}
	out.resize(image_size);
	ret = spng_decode_image(ctx, out.data(), image_size, SPNG_FMT_RGBA8, 0);
	spng_ctx_free(ctx);
	fclose(f);
	if (ret) {
		std::cerr << "atlas_autocut: decode_image error " << ret << "\n";
		return false;
	}
	return true;
}

int main(int argc, char** argv)
{
	if (argc < 2) {
		std::cerr << "Usage: atlas_autocut <input.png> [alpha_threshold=16] [min_w=12] [min_h=12] [pad=2]\n";
		return 1;
	}
	std::string input = argv[1];
	int alpha_thresh = argc > 2 ? std::atoi(argv[2]) : 16;
	int min_w = argc > 3 ? std::atoi(argv[3]) : 12;
	int min_h = argc > 4 ? std::atoi(argv[4]) : 12;
	int pad = argc > 5 ? std::atoi(argv[5]) : 2;

	std::vector<uint8_t> rgba;
	int W = 0, H = 0;
	if (!load_png_rgba8(input, rgba, W, H)) return 1;

	auto alpha_at = [&](int x, int y) -> uint8_t {
		size_t idx = (size_t)(y * W + x) * 4 + 3;
		return rgba[idx];
	};

	std::vector<uint8_t> visited((size_t)W * H, 0);
	std::vector<Rect> rects;
	std::queue<std::pair<int,int>> q;

	for (int y = 0; y < H; ++y) {
		for (int x = 0; x < W; ++x) {
			size_t id = (size_t)y * W + x;
			if (visited[id]) continue;
			if (alpha_at(x, y) <= alpha_thresh) continue;
			// BFS
			int minx = x, miny = y, maxx = x, maxy = y;
			visited[id] = 1;
			q.push({x,y});
			while (!q.empty()) {
				auto [cx, cy] = q.front(); q.pop();
				minx = std::min(minx, cx);
				maxx = std::max(maxx, cx);
				miny = std::min(miny, cy);
				maxy = std::max(maxy, cy);
				static const int dx[4] = {1,-1,0,0};
				static const int dy[4] = {0,0,1,-1};
				for (int k = 0; k < 4; ++k) {
					int nx = cx + dx[k], ny = cy + dy[k];
					if ((unsigned)nx >= (unsigned)W || (unsigned)ny >= (unsigned)H) continue;
					size_t nid = (size_t)ny * W + nx;
					if (visited[nid]) continue;
					if (alpha_at(nx, ny) <= alpha_thresh) continue;
					visited[nid] = 1;
					q.push({nx, ny});
				}
			}
			// Expand padding and clamp
			minx = std::max(0, minx - pad);
			miny = std::max(0, miny - pad);
			maxx = std::min(W - 1, maxx + pad);
			maxy = std::min(H - 1, maxy + pad);
			int rw = maxx - minx + 1;
			int rh = maxy - miny + 1;
			if (rw >= min_w && rh >= min_h) {
				rects.push_back({minx, miny, rw, rh});
			}
		}
	}

	// Sort rects for consistent output
	std::sort(rects.begin(), rects.end(), [](const Rect& a, const Rect& b){
		if (a.y != b.y) return a.y < b.y;
		return a.x < b.x;
	});

	// Write JSON next to input
	std::string out_json = input;
	size_t dot = out_json.find_last_of('.');
	if (dot != std::string::npos) out_json = out_json.substr(0, dot);
	out_json += "_autocut.json";

	nlohmann::json j;
	j["atlas"] = input;
	j["image_width"] = W;
	j["image_height"] = H;
	j["items"] = nlohmann::json::array();
	for (size_t i = 0; i < rects.size(); ++i) {
		const auto& r = rects[i];
		nlohmann::json it;
		it["name"] = "cut_" + std::to_string(i);
		it["rect"] = { r.x, r.y, r.w, r.h };
		j["items"].push_back(it);
	}

	std::ofstream ofs(out_json);
	if (!ofs) {
		std::cerr << "atlas_autocut: Failed to write " << out_json << "\n";
		return 1;
	}
	ofs << j.dump(2) << "\n";
	std::cout << "atlas_autocut: Wrote " << out_json << " with " << rects.size() << " rects\n";
	return 0;
}

