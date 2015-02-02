#include "gentex.hpp"

#include <fstream>
#include <iostream>
#include <algorithm>

#include "calculate/calculate.hpp"
extern "C" {
#include "shunting-yard/shunting-yard.h"
}

namespace gentex {

inline float rnd() { return rand() / (float)RAND_MAX; }

inline Color parseColor(const Json& param, Color def = Color(1.f)) {
	if (param.is_array()) {
		const auto& arr = param.array_items();
		return Color(arr[0].number_value(), arr[1].number_value(), arr[2].number_value());
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
		} else if (str[0] == '=') {
			postfix_t postfix = infix2postfix(str.substr(1));
			float res = evalpostfix(postfix);
			return Color(res);
		} else if (str[0] == '?') {
			double res = 0;
			shunting_yard(str.substr(1).c_str(), &res);
			return Color(res);
		}
	}
	return def;
}

inline Color parseColor(const char* name, const Json& params, Color def = Color(1.f)) {
	return parseColor(params[name], def);
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

struct ColorInterpolator {
	struct GradientPoint { float pos; Color color; };

	ColorInterpolator(const Json& params) {
		if (params["colors"].is_array()) {
			const auto& colors = params["colors"].array_items();
			for (uint i = 0; i < colors.size(); ++i) {
				points.push_back({
					i / (colors.size() - 1.f),
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
		const std::string& str = params["expr"].string_value();
		Color tint = parseColor("tint", params);
		dst.composite([=](int, int) {
			postfix_t postfix = infix2postfix(str);
			float res = evalpostfix(postfix);
			return Color(res) * tint;
		}, op);
	}},
	{ "shun", [](Image& dst, CompositeFunction op, const Json& params) {
		const std::string& str = params["expr"].string_value();
		Color tint = parseColor("tint", params);
		dst.composite([=](int, int) {
			double res = 0;
			shunting_yard(str.c_str(), &res);
			return Color(res) * tint;
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


void initMathParser() {
	opers.insert({"+", oper_t{false, 1, false}});
	opers.insert({"-", oper_t{false, 1, false}});
	opers.insert({"*", oper_t{false, 2, false}});
	opers.insert({"/", oper_t{false, 2, false}});
	opers.insert({"%", oper_t{false, 2, false}});
	opers.insert({"^", oper_t{true, 3, false}});
	opers.insert({"+", oper_t{false, 10, true}});
	opers.insert({"-", oper_t{false, 10, true}});
	opers.insert({"!", oper_t{true, 11, true}});

	funcs.insert({"+", func_args(1, [](args_t v) { return v[0]; })});
	funcs.insert({"+", func_args(2, [](args_t v) { return v[0] + v[1]; })});
	funcs.insert({"-", func_args(1, [](args_t v) { return -v[0]; })});
	funcs.insert({"-", func_args(2, [](args_t v) { return v[0] - v[1]; })});
	funcs.insert({"*", func_args(2, [](args_t v) { return v[0] * v[1]; })});
	funcs.insert({"/", func_args(2, [](args_t v) { return v[0] / v[1]; })});
	funcs.insert({"%", func_args(2, [](args_t v) { return fmod(v[0], v[1]); })});
	funcs.insert({"^", func_args(2, [](args_t v) { return pow(v[0], v[1]); })});
	funcs.insert({"abs", func_args(1, [](args_t v) { return std::abs(v[0]); })});
	funcs.insert({"log", func_args(1, [](args_t v) { return log10(v[0]); })});
	funcs.insert({"log", func_args(2, [](args_t v) { return log(v[1]) / log(v[0]); })});
	funcs.insert({"ln", func_args(1, [](args_t v) { return log(v[0]); })});
	funcs.insert({"sqrt", func_args(1, [](args_t v) { return sqrt(v[0]); })});
	funcs.insert({"root", func_args(2, [](args_t v) { return pow(v[1], 1.0 / v[0]); })});
	funcs.insert({"sin", func_args(1, [](args_t v) { return sin(v[0]); })});
	funcs.insert({"cos", func_args(1, [](args_t v) { return cos(v[0]); })});
	funcs.insert({"tan", func_args(1, [](args_t v) { return tan(v[0]); })});
	funcs.insert({"asin", func_args(1, [](args_t v) { return asin(v[0]); })});
	funcs.insert({"acos", func_args(1, [](args_t v) { return acos(v[0]); })});
	funcs.insert({"atan", func_args(1, [](args_t v) { return atan(v[0]); })});
	funcs.insert({"atan2", func_args(2, [](args_t v) { return atan2(v[0], v[1]); })});
	funcs.insert({"ceil", func_args(1, [](args_t v) { return ceil(v[0]); })});
	funcs.insert({"floor", func_args(1, [](args_t v) { return floor(v[0]); })});
	funcs.insert({"min", [](args_t v) {
		if (v.size() > 0) return return_t(true, *min_element(v.begin(), v.end()));
		else return return_t(false, 0.0);
	}});
	funcs.insert({"max", [](args_t v)
	{
		if (v.size() > 0) return return_t(true, *max_element(v.begin(), v.end()));
		else return return_t(false, 0.0);
	}});
	funcs.insert({"!", func_args(1, [](args_t v) { return tgamma(v[0] + 1); })});
	funcs.insert({"pi", func_constant(acos(-1.L))});
	funcs.insert({"e", func_constant(exp(1.L))});
	funcs.insert({"_", func_constant(NAN)});
}

} // namespace
