
#include <string>
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>

#include "gentex.hpp"

using namespace gentex;
using namespace json11;

const std::vector<Op> s_ops = {
	{ "set", [](Color  , Color b) { return b; } },
	{ "add", [](Color a, Color b) { return a + b; } },
	{ "sub", [](Color a, Color b) { return a - b; } },
	{ "mul", [](Color a, Color b) { return a * b; } },
	{ "div", [](Color a, Color b) { return a / b; } },
	{ "min", [](Color a, Color b) { return min(a, b); } },
	{ "max", [](Color a, Color b) { return max(a, b); } },
};

std::string readFile(const std::string& path) {
	std::ifstream f(path);
	return std::string(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
}

inline void msleep(int ms) { std::this_thread::sleep_for(std::chrono::milliseconds(ms)); }

void panic(const char* msg) {
	std::cout << msg << std::endl;
	exit(1);
}

bool doTexture(const Json& spec) {
	const std::string& outfile = spec["out"].string_value();
	std::cout << "Generating " << outfile << "..." << std::flush;
	auto t0 = std::chrono::steady_clock::now();
	int w = spec["size"][0].int_value();
	int h = spec["size"][1].int_value();
	Image tex(w, h);

	const auto& cmds = spec["ops"].array_items();
	for (const auto& cmd : cmds) {
		for (const auto& op : s_ops) {
			if (!cmd[op.name].is_null()) {
				const std::string& gen = cmd[op.name].string_value();
				//std::cout << "Applying " << gen << " with " << op.name << std::endl;
				getCommand(gen)(tex, op.op, cmd);
				break;
			}
		}
	}

	auto t1 = std::chrono::steady_clock::now();
	auto dtms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
	std::cout << " " << dtms << " ms" << std::flush;
	tex.writeTGA(outfile);
	auto t2 = std::chrono::steady_clock::now();
	dtms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
	std::cout << "   (write: " << dtms << " ms)" << std::endl;
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
		if (arg == "-h" || arg == "--help") {
			std::cout << "USAGE: " << argv[0] << " [-w | --watch] FILE1 [FILE2...]" << std::endl;
			return 0;
		}
		else if (arg == "-w" || arg == "--watch") {
			watch = true;
		}
		else paths.push_back(arg);
	}
	if (paths.empty())
		panic("Specify input file");

	initMathParser();

	int failCount = 0;
	std::vector<std::string> texts;
	for (const auto& path: paths) {
		std::cout << "Processing " << path << "..." << std::endl;
		texts.push_back(readFile(path));
		auto t0 = std::chrono::steady_clock::now();
		if (!doScript(texts.back()))
			failCount++;
		auto t1 = std::chrono::steady_clock::now();
		auto dtms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
		std::cout << "File done in " << dtms << " ms" << std::endl;
	}
	if (!watch)
		return failCount;

	while (true) {
		msleep(500);
		// TODO: This is very crappy way to detect changes
		for (unsigned i = 0; i < paths.size(); ++i) {
			std::string newText = readFile(paths[i]);
			if (texts[i] != newText) {
				std::cout << "Reprocessing " << paths[i] << "..." << std::endl;
				auto t0 = std::chrono::steady_clock::now();
				doScript(newText);
				auto t1 = std::chrono::steady_clock::now();
				auto dtms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
				std::cout << "File done in " << dtms << " ms" << std::endl;
				texts[i] = newText;
			}
		}
	}

	return 0;
}
