#include "gentex.hpp"

#include <fstream>
#include <iostream>
#include <algorithm>

#include "shunting-yard-cpp/shunting-yard.hpp"

namespace gentex {

inline float rnd() { return rand() / (float)RAND_MAX; }

inline float parseFloat(const Json& param, float def = 0.f) {
	if (param.is_number())
		return param.number_value();
	else if (param.is_string())
		return calc::MathExpression::eval(param.string_value());
	return def;
}

inline float parseFloat(const char* name, const Json& params, float def = 0.f) {
	return parseFloat(params[name], def);
}

inline vec2 parseVec2(const char* name, const Json& params, vec2 def = vec2(0.f)) {
	const Json& param = params[name];
	if (param.is_array()) {
		const auto& arr = param.array_items();
		return vec2(parseFloat(arr[0], def.x), parseFloat(arr[1], def.y));
	} else if (param.is_number() || param.is_string())
		return vec2(parseFloat(param, def.x));
	return def;
}

inline Color parseColor(const Json& param, Color def = Color(1.f)) {
	if (param.is_array()) {
		const auto& arr = param.array_items();
		return Color(parseFloat(arr[0]), parseFloat(arr[1]), parseFloat(arr[2]));
	} else if (param.is_number()) {
		return Color(param.number_value());
	} else if (param.is_string()) {
		const std::string& str = param.string_value();
		if (str.empty()) return def;
		if (str.length() == 7 && str[0] == '#') {
			float r = std::stoi(str.substr(1, 2), 0, 16) / 255.f;
			float g = std::stoi(str.substr(3, 2), 0, 16) / 255.f;
			float b = std::stoi(str.substr(5, 2), 0, 16) / 255.f;
			return Color(r, g, b);
		} else if (str.length() == 4 && str[0] == '#') {
			float r = std::stoi(str.substr(1, 1) + str.substr(1, 1), 0, 16) / 255.f;
			float g = std::stoi(str.substr(2, 1) + str.substr(2, 1), 0, 16) / 255.f;
			float b = std::stoi(str.substr(3, 1) + str.substr(3, 1), 0, 16) / 255.f;
			return Color(r, g, b);
		} else if (str[0] == '#') {
			std::cerr << "malformed hex color string \"" << str << "\"" << std::endl;
		} else {
			return Color(calc::MathExpression::eval(str));
		}
	}
	return def;
}

inline Color parseColor(const char* name, const Json& params, Color def = Color(1.f)) {
	return parseColor(params[name], def);
}

struct ColorInterpolator {
	struct GradientPoint { float pos; Color color; };

	ColorInterpolator(const Json& params) {
		// TODO: More error checking
		if (params["colors"].is_array()) {
			const auto& colors = params["colors"].array_items();
			float fstops[colors.size()];
			for (uint i = 0; i < colors.size(); ++i)
				fstops[i] = i / (colors.size() - 1.f);
			if (params["stops"].is_array()) {
				const auto& stops = params["stops"].array_items();
				if (stops.size() == colors.size() - 2) {
					for (uint i = 1; i < colors.size()-1; ++i)
						fstops[i] = stops[i - 1].number_value();
				} else std::cerr << "malformed gardient stop array, should have " << (colors.size() - 2) << " elements" << std::endl;
			}
			for (uint i = 0; i < colors.size(); ++i) {
				points.push_back({
					fstops[i],
					parseColor(colors[i])
				});
			}
		} else std::cerr << "malformed gradient color array" << std::endl;
	}

	Color get(Color pos) {
		// TODO: Repeat?
		// TODO: Handle all components separately
		uint i = 0;
		while (i < points.size()-1 && points[i+1].pos < pos.r) ++i;
		GradientPoint& p1 = points[i];
		GradientPoint& p2 = points[i+1];
		float alpha = (pos.r - p1.pos) / (p2.pos - p1.pos);
		return mix(p1.color, p2.color, alpha);
	}

	std::vector<GradientPoint> points;
};


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
	{ "turbulence", [](Image& dst, CompositeFunction op, const Json& params) {
		float s = parseFloat("size", params, 1.f) * min(dst.w, dst.h);
		Color tint = parseColor("tint", params);
		dst.composite([=](int x, int y) {
			float value = 0;
			float size = s;
			while (size >= 1.f) {
				value += perlin(vec2(x / size, y / size)) * size;
				size *= 0.5f;
			}
			return Color(value / s * 0.5f + 0.5f) * tint;
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
	{ "gradient", [](Image& dst, CompositeFunction op, const Json& params) {
		ColorInterpolator interp(params);
		dst.filter([&](int, int, Color color) {
			return interp.get(color);
		}, op);
	}},
	{ "sin", [](Image& dst, CompositeFunction op, const Json& params) {
		vec2 freq = parseVec2("freq", params, vec2(1.f)) * (float)M_PI;
		vec2 offset = parseVec2("offset", params, vec2(0.f));
		Color tint = parseColor("tint", params);
		dst.composite([=](int x, int y) {
			vec2 s = sin((vec2(x, y) + offset) * freq);
			return op(s.x * tint, s.y * tint);
		}, op);
	}},
	{ "sinx", [](Image& dst, CompositeFunction op, const Json& params) {
		float freq = params["freq"].number_value() * M_PI;
		float offset = params["offset"].number_value();
		Color tint = parseColor("tint", params);
		dst.composite([=](int x, int) {
			return std::sin((x + offset) * freq) * tint;
		}, op);
	}},
	{ "siny", [](Image& dst, CompositeFunction op, const Json& params) {
		float freq = params["freq"].number_value() * M_PI;
		float offset = params["offset"].number_value();
		Color tint = parseColor("tint", params);
		dst.composite([=](int, int y) {
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
	{ "calc", [](Image& dst, CompositeFunction op, const Json& params) {
		Color tint = parseColor("tint", params);
		double w = dst.w, h = dst.h;
		const Json& exprParam = params["expr"];
		if (exprParam.is_string()) {
			calc::MathExpression expr(exprParam.string_value());
			expr.setVar('w', w);
			expr.setVar('h', h);
			dst.composite([=, &expr](int x, int y) {
				expr.setVar('x', x);
				expr.setVar('y', y);
				return Color(expr.eval()) * tint;
			}, op);
		} else if (exprParam.is_array()) {
			calc::MathExpression r(exprParam.array_items()[0].string_value());
			calc::MathExpression g(exprParam.array_items()[1].string_value());
			calc::MathExpression b(exprParam.array_items()[2].string_value());
			r.setVar('w', w); g.setVar('w', w); b.setVar('w', w);
			r.setVar('h', h); g.setVar('h', h); b.setVar('h', h);
			dst.composite([=, &r, &g, &b](int x, int y) {
				r.setVar('x', x); g.setVar('x', x); b.setVar('x', x);
				r.setVar('y', y); g.setVar('y', y); b.setVar('y', y);
				return Color(r.eval(), g.eval(), b.eval()) * tint;
			}, op);
		}
	}},
};

CommandFunction& getCommand(const std::string& name) {
	return s_cmds[name];
}

void initMathParser() {
	calc::MathExpression::funcs.push_back({"perlin", [](double x)->double{ return perlin(vec2(x, 0.f)) * 0.5f + 0.5f; }});
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
	std::vector<char> pixbuf;
	pixbuf.resize(w * h * 3);
	// Image data
	int i = 0;
	for (int y = h-1; y >= 0; --y) {
		for (int x = 0; x < w; ++x) {
			const Color pix = saturate(get(x, y));
			pixbuf[i++] = static_cast<unsigned char>(pix.b * 255);
			pixbuf[i++] = static_cast<unsigned char>(pix.g * 255);
			pixbuf[i++] = static_cast<unsigned char>(pix.r * 255);
		}
	}
	tgaout.write(&pixbuf[0], pixbuf.size());
}

} // namespace
