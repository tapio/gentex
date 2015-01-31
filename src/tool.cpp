
#include <string>
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>

#include "gentex.hpp"
#include "json11/json11.hpp"

using namespace gentex;
using namespace json11;

struct Op {
	std::string name;
	CompositeFunction op;
};

const std::vector<Op> s_ops = {
	{ "set", [](Color  , Color b) { return b; } },
	{ "add", [](Color a, Color b) { return a + b; } },
	{ "sub", [](Color a, Color b) { return a - b; } },
	{ "mul", [](Color a, Color b) { return a * b; } },
	{ "div", [](Color a, Color b) { return a / b; } },
	{ "min", [](Color a, Color b) { return min(a, b); } },
	{ "max", [](Color a, Color b) { return max(a, b); } },
};

inline Color parseTint(const Json& params) {
	if (params["tint"].is_array()) {
		const auto& arr = params["tint"].array_items();
		return Color(arr[0].number_value(), arr[1].number_value(), arr[2].number_value());
	} else return Color(1.0f);
}

inline void msleep(int ms) { std::this_thread::sleep_for(std::chrono::milliseconds(ms)); }
inline float rnd() { return rand() / (float)RAND_MAX; }


typedef std::function<void(Image&, CompositeFunction, const Json&)> CommandFunction;

struct Command {
	std::string name;
	CommandFunction cmd;
};

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
};

std::string readFile(const std::string& path) {
	std::ifstream f(path);
	return std::string(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
}

void panic(const char* msg) {
	std::cout << msg << std::endl;
	exit(1);
}

bool doTexture(const Json& spec) {
	const std::string& outfile = spec["out"].string_value();
	std::cout << "Generating " << outfile << "..." << std::endl;
	int w = spec["size"][0].int_value();
	int h = spec["size"][1].int_value();
	Image tex(w, h);

	const auto& cmds = spec["ops"].array_items();
	for (const auto& cmd : cmds) {
		for (const auto& op : s_ops) {
			if (!cmd[op.name].is_null()) {
				const std::string& gen = cmd[op.name].string_value();
				std::cout << "Applying " << gen << " with " << op.name << std::endl;
				s_cmds[gen](tex, op.op, cmd);
				break;
			}
		}
	}

	tex.writeTGA(outfile);
	return true;
}

bool doScript(const std::string& text) {
	std::string err;
	Json specs = Json::parse(text, err);
	if (!err.empty()) {
		std::cerr << err << std::endl;
		return false;
	}

	if (specs.is_object())
		return doTexture(specs);

	bool allFine = true;
	for (const auto& spec : specs.array_items())
		allFine &= doTexture(spec);

	return allFine;
}

int main(int argc, char** argv) {
	std::vector<std::string> paths;
	bool watch = false;
	for (int i = 1; i < argc; ++i) {
		std::string arg = argv[i];
		if (arg == "-w" || arg == "--watch") {
			watch = true;
		}
		else paths.push_back(arg);
	}
	if (paths.empty())
		panic("specify input file");

	if (!watch) {
		int ret = 0;
		for (const auto& path: paths) {
			std::cout << "Processing " << path << "..." << std::endl;
			if (!doScript(readFile(path)))
				ret++;
		}
		return ret;
	}

	std::vector<std::string> texts;
	for (const auto& path: paths) {
		texts.push_back(readFile(path));
	}

	while (true) {
		msleep(500);
		// TODO: This is very crappy way to detect changes
		for (unsigned i = 0; i < paths.size(); ++i) {
			std::string newText = readFile(paths[i]);
			if (texts[i] != newText) {
				std::cout << "Reprocessing " << paths[i] << "..." << std::endl;
				doScript(newText);
				texts[i] = newText;
			}
		}
	}

	return 0;
}
