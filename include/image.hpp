#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <functional>

#define GLM_SWIZZLE
#define GLM_FORCE_CXX11
#include <glm/glm.hpp>

using glm::vec3;

class Image {
public:
	typedef std::function<vec3(int, int, vec3)> FilterFunction;
	typedef std::function<vec3(int, int)> GeneratorFunction;

	Image(int w, int h): w(w), h(h), buffer(w * h) { }

	vec3 sample(float u, float v) {
		return get(u / w, v / h);
	};

	vec3 sampleClamp(float u, float v) {
		u = u < 0 ? 0 : (u > 1.0 ? 1.0 : u);
		v = v < 0 ? 0 : (v > 1.0 ? 1.0 : v);
		return get(u / w, v / h);
	};

	vec3 sampleRepeat(float u, float v) {
		return get(int(u / w) % w, int(v / h) % h);
	};

	vec3 get(int x, int y) {
		return buffer[x + y * w];
	};

	vec3 getClamp(int x, int y) {
		x = x < 0 ? 0 : (x > w ? w : x);
		y = y < 0 ? 0 : (y > h ? h : y);
		return get(x, y);
	};

	vec3 getRepeat(int x, int y) {
		return get(x % w, y % h);
	};

	void filter(FilterFunction func) {
		for (int y = 0; y < h; ++y) {
			for (int x = 0; x < w; ++x) {
				vec3& color = buffer[y * w + x];
				color = func(x, y, color);
			}
		}
	}

	void generate(GeneratorFunction func) {
		for (int y = 0; y < h; ++y) {
			for (int x = 0; x < w; ++x) {
				buffer[y * w + x] = func(x, y);
			}
		}
	}

	void writeTGA(std::string filepath = "out.tga") {
		std::ofstream tgaout(filepath.c_str(), std::ios::binary);
		char tga_header_part1[] = {
			0x00,  // No id field
			0x00,  // No palette
			0x02,  // 2 = Uncompressed true-color
			0x00, 0x00, 0x00, 0x00, 0x00,  // Palette stuff (not used)
			0x00, 0x00,  // X-origin
			0x00, 0x00  // Y-origin
		};
		tgaout.write(tga_header_part1, sizeof(tga_header_part1));
		// 16-bit width and height (little-endian)
		tgaout << static_cast<char>(w & 0xff);
		tgaout << static_cast<char>((w >> 8) & 0xff);
		tgaout << static_cast<char>(h & 0xff);
		tgaout << static_cast<char>((h >> 8) & 0xff);
		tgaout << static_cast<char>(24); // Bits per pixel
		tgaout << static_cast<char>(0x00);  // No special flags
		// Image data
		for (int y = h-1; y >= 0; --y) {
			for (int x = 0; x < w; ++x) {
				const vec3& pix = get(x, y);
				tgaout << static_cast<char>(pix.b * 255);
				tgaout << static_cast<char>(pix.g * 255);
				tgaout << static_cast<char>(pix.r * 255);
			}
		}
	}

	int w, h;
	std::vector<vec3> buffer;
};
