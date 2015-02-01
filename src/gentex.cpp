#include "gentex.hpp"

#include <fstream>
#include <iostream>

namespace gentex {

inline float rnd() { return rand() / (float)RAND_MAX; }

inline Color parseColor(const char* name, const Json& params, Color def = Color(1.f)) {
	const Json& param = params[name];
	if (param.is_array()) {
		const auto& arr = param.array_items();
		return Color(arr[0].number_value(), arr[1].number_value(), arr[2].number_value());
	} else if (param.is_number()) {
		return Color(param.number_value());
	} else if (param.is_string()) {
		const std::string& hex = param.string_value();
		if (hex.length() == 7 && hex[0] == '#') {
			float r = std::stoi(hex.substr(1, 2), 0, 16) / 255.f;
			float g = std::stoi(hex.substr(3, 2), 0, 16) / 255.f;
			float b = std::stoi(hex.substr(5, 2), 0, 16) / 255.f;
			return Color(r, g, b);
		} else if (hex.length() == 4 && hex[0] == '#') {
			float r = std::stoi(hex.substr(1, 1) + hex.substr(1, 1), 0, 16) / 255.f;
			float g = std::stoi(hex.substr(2, 1) + hex.substr(2, 1), 0, 16) / 255.f;
			float b = std::stoi(hex.substr(3, 1) + hex.substr(3, 1), 0, 16) / 255.f;
			return Color(r, g, b);
		}
		std::cerr << "malformed hex color string \"" << hex << "\"" << std::endl;
	}
	return def;
}

inline vec2 parseVec2(const char* name, const Json& params, vec2 def = vec2(0.f)) {
	const Json& param = params[name];
	if (param.is_array()) {
		const auto& arr = param.array_items();
		return vec2(arr[0].number_value(), arr[1].number_value());
	} else if (param.is_number())
		return vec2(param.number_value());
	return def;
}

inline float parseFloat(const char* name, const Json& params, float def = 0.f) {
	const Json& param = params[name];
	return param.is_number() ? param.number_value() : def;
}

std::map<std::string, CommandFunction> s_cmds = {
	{ "const", [](Image& dst, CompositeFunction op, const Json& params) {
		Color tint = parseColor("tint", params);
		dst.composite([tint](int, int) {
			return tint;
		}, op);
	}},
	{ "noise", [](Image& dst, CompositeFunction op, const Json& params) {
		Color tint = parseColor("tint", params);
		dst.composite([tint](int, int) {
			return Color(rnd()) * tint;
		}, op);
	}},
	{ "simplex", [](Image& dst, CompositeFunction op, const Json& params) {
		vec2 freq = parseVec2("freq", params, vec2(1.f));
		vec2 offset = parseVec2("offset", params, vec2(0.f));
		Color tint = parseColor("tint", params);
		dst.composite([freq, offset, tint](int x, int y) {
			return Color(simplex((vec2(x, y) + offset) * freq) * 0.5f + 0.5f) * tint;
		}, op);
	}},
	{ "perlin", [](Image& dst, CompositeFunction op, const Json& params) {
		vec2 freq = parseVec2("freq", params, vec2(1.f));
		vec2 offset = parseVec2("offset", params, vec2(0.f));
		vec2 period = vec2(dst.w, dst.h) * freq;
		Color tint = parseColor("tint", params);
		dst.composite([freq, offset, period, tint](int x, int y) {
			return Color(perlin((vec2(x, y) + offset) * freq, period) * 0.5f + 0.5f) * tint;
		}, op);
	}},
	{ "fbm", [](Image& dst, CompositeFunction op, const Json& params) {
		vec2 freq = parseVec2("freq", params, vec2(1.f));
		vec2 offset = parseVec2("offset", params, vec2(0.f));
		float octaves = parseFloat("octaves", params, 1.f);
		float persistence = parseFloat("persistence", params, 0.5f);
		float lacunarity = parseFloat("lacunarity", params, 2.0f);
		Color tint = parseColor("tint", params);
		dst.composite([=](int x, int y) {
			float c = 0.0f;
			float amplitude = 1.0f;
			vec2 f = freq;
			vec2 pos = vec2(x, y) + offset;
			for (int i = 0; i < octaves; ++i) {
				c += (perlin(pos * f)) * amplitude;
				amplitude *= persistence;
				f *= lacunarity;
			}
			return Color(c * 0.5f + 0.5f) * tint;
		}, op);
	}},
	{ "pow", [](Image& dst, CompositeFunction op, const Json& params) {
		float density = 1.f - params["density"].number_value();
		float sharpness = params["sharpness"].number_value();
		Color tint = parseColor("tint", params);
		dst.filter([density, sharpness, tint](int, int, Color color) {
			Color c = max(color - density, Color(0.f));
			return (1.f - glm::pow(Color(sharpness), c)) * tint;
		}, op);
	}},
	{ "inv", [](Image& dst, CompositeFunction op, const Json& params) {
		Color tint = parseColor("tint", params);
		dst.filter([tint](int, int, Color color) {
			return (1.f - color) * tint;
		}, op);
	}},
	{ "clamp", [](Image& dst, CompositeFunction op, const Json&) {
		dst.filter([](int, int, Color color) {
			return saturate(color);
		}, op);
	}},
	{ "lerp", [](Image& dst, CompositeFunction op, const Json& params) {
		Color a = parseColor("a", params, Color(0.f));
		Color b = parseColor("b", params, Color(1.f));
		dst.filter([a, b](int, int, Color color) {
			return mix(a, b, color);
		}, op);
	}},
	{ "sinx", [](Image& dst, CompositeFunction op, const Json& params) {
		float freq = params["freq"].number_value() * M_PI;
		float offset = params["offset"].number_value();
		Color tint = parseColor("tint", params);
		dst.composite([freq, offset, tint](int x, int) {
			return std::sin((x + offset) * freq) * tint;
		}, op);
	}},
	{ "siny", [](Image& dst, CompositeFunction op, const Json& params) {
		float freq = params["freq"].number_value() * M_PI;
		float offset = params["offset"].number_value();
		Color tint = parseColor("tint", params);
		dst.composite([freq, offset, tint](int, int y) {
			return std::sin((y + offset) * freq) * tint;
		}, op);
	}},
	{ "or", [](Image& dst, CompositeFunction op, const Json& params) {
		float w = dst.w;
		Color tint = parseColor("tint", params);
		dst.composite([w, tint](int x, int y) {
			return ((x | y) / w) * tint;
		}, op);
	}},
	{ "xor", [](Image& dst, CompositeFunction op, const Json& params) {
		float w = dst.w;
		Color tint = parseColor("tint", params);
		dst.composite([w, tint](int x, int y) {
			return ((x ^ y) / w) * tint;
		}, op);
	}},
	{ "rect", [](Image& dst, CompositeFunction op, const Json& params) {
		vec2 pos = parseVec2("pos", params);
		vec2 size = parseVec2("size", params);
		Color tint = parseColor("tint", params);
		dst.composite([pos, size, tint](int x, int y) {
			float c = x >= pos.x && x < pos.x + size.x && y >= pos.y && y < pos.y + size.y ? 1.f : 0.f;
			return Color(c) * tint;
		}, op);
	}},
	{ "circle", [](Image& dst, CompositeFunction op, const Json& params) {
		vec2 pos = parseVec2("pos", params, vec2(dst.w * 0.5f, dst.h * 0.5f));
		float r = params["radius"].number_value();
		Color tint = parseColor("tint", params);
		dst.composite([pos, r, tint](int x, int y) {
			float c = glm::distance(pos, vec2(x, y)) <= r ? 1.f : 0.f;
			return Color(c) * tint;
		}, op);
	}},
};

CommandFunction& getCommand(const std::string& name) {
	return s_cmds[name];
}


// Image class

void Image::writeTGA(const std::string& filepath) const {
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
			const Color pix = saturate(get(x, y));
			tgaout << static_cast<unsigned char>(pix.b * 255);
			tgaout << static_cast<unsigned char>(pix.g * 255);
			tgaout << static_cast<unsigned char>(pix.r * 255);
		}
	}
}

} // namespace
