#include "gentex.hpp"

#include <fstream>
#include <iostream>

namespace gentex {

inline float rnd() { return rand() / (float)RAND_MAX; }

inline Color parseTint(const Json& params) {
	const Json& param = params["tint"];
	if (param.is_array()) {
		const auto& arr = param.array_items();
		return Color(arr[0].number_value(), arr[1].number_value(), arr[2].number_value());
	} else if (param.is_number())
		return Color(param.number_value());
	return Color(1.0f);
}

inline vec2 parseVec2(const char* name, const Json& params, float def = 0) {
	const Json& param = params[name];
	if (param.is_array()) {
		const auto& arr = param.array_items();
		return vec2(arr[0].number_value(), arr[1].number_value());
	} else if (param.is_number())
		return vec2(param.number_value());
	return vec2(def);
}

std::map<std::string, CommandFunction> s_cmds = {
	{ "const", [](Image& dst, CompositeFunction op, const Json& params) {
		Color tint = parseTint(params);
		dst.composite([tint](int, int) {
			return tint;
		}, op);
	}},
	{ "noise", [](Image& dst, CompositeFunction op, const Json& params) {
		Color tint = parseTint(params);
		dst.composite([tint](int, int) {
			return Color(rnd()) * tint;
		}, op);
	}},
	{ "simplex", [](Image& dst, CompositeFunction op, const Json& params) {
		vec2 freq = parseVec2("freq", params, 1.f);
		vec2 offset = parseVec2("offset", params, 0.f);
		Color tint = parseTint(params);
		dst.composite([freq, offset, tint](int x, int y) {
			return Color(simplex((vec2(x, y) + offset) * freq) * 0.5f + 0.5f) * tint;
		}, op);
	}},
	{ "perlin", [](Image& dst, CompositeFunction op, const Json& params) {
		vec2 freq = parseVec2("freq", params, 1.f);
		vec2 offset = parseVec2("offset", params, 0.f);
		Color tint = parseTint(params);
		dst.composite([freq, offset, tint](int x, int y) {
			return Color(perlin((vec2(x, y) + offset) * freq) * 0.5f + 0.5f) * tint;
		}, op);
	}},
	{ "sinx", [](Image& dst, CompositeFunction op, const Json& params) {
		float freq = params["freq"].number_value() * M_PI;
		float offset = params["offset"].number_value();
		Color tint = parseTint(params);
		dst.composite([freq, offset, tint](int x, int) {
			return std::sin((x + offset) * freq) * tint;
		}, op);

	}},
	{ "siny", [](Image& dst, CompositeFunction op, const Json& params) {
		float freq = params["freq"].number_value() * M_PI;
		float offset = params["offset"].number_value();
		Color tint = parseTint(params);
		dst.composite([freq, offset, tint](int, int y) {
			return std::sin((y + offset) * freq) * tint;
		}, op);
	}},
	{ "or", [](Image& dst, CompositeFunction op, const Json& params) {
		float w = dst.w;
		Color tint = parseTint(params);
		dst.composite([w, tint](int x, int y) {
			return ((x | y) / w) * tint;
		}, op);
	}},
	{ "xor", [](Image& dst, CompositeFunction op, const Json& params) {
		float w = dst.w;
		Color tint = parseTint(params);
		dst.composite([w, tint](int x, int y) {
			return ((x ^ y) / w) * tint;
		}, op);
	}},
	{ "rect", [](Image& dst, CompositeFunction op, const Json& params) {
		vec2 pos = parseVec2("pos", params);
		vec2 size = parseVec2("size", params);
		Color tint = parseTint(params);
		dst.composite([pos, size, tint](int x, int y) {
			float c = x >= pos.x && x < pos.x + size.x && y >= pos.y && y < pos.y + size.y ? 1.f : 0.f;
			return Color(c) * tint;
		}, op);
	}},
	{ "circle", [](Image& dst, CompositeFunction op, const Json& params) {
		vec2 pos = parseVec2("pos", params, dst.w * 0.5f);
		float r = params["radius"].number_value();
		Color tint = parseTint(params);
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
