
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cctype>

#include "gentex.hpp"
#include "json11/json11.hpp"

using namespace gentex;
using namespace json11;

typedef std::function<vec4(vec4, vec4)> CompositeFunction;

struct Op {
	std::string name;
	CompositeFunction op;
};

const std::vector<Op> s_ops = {
	{ "set", [](vec4  , vec4 b) { return b; } },
	{ "add", [](vec4 a, vec4 b) { return a + b; } },
	{ "sub", [](vec4 a, vec4 b) { return a - b; } },
	{ "mul", [](vec4 a, vec4 b) { return a * b; } },
	{ "div", [](vec4 a, vec4 b) { return a / b; } },
	{ "min", [](vec4 a, vec4 b) { return min(a, b); } },
	{ "max", [](vec4 a, vec4 b) { return max(a, b); } },
};


std::string readFile(const std::string& path) {
	std::ifstream f(path);
	return std::string(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
}

void panic(const char* msg) {
	std::cout << msg << std::endl;
	exit(1);
}

int main(int /*argc*/, char** argv) {
	std::string err;
	Json spec = Json::parse(readFile(argv[1]), err);
	if (!err.empty())
		panic(err.c_str());

	int w = spec["size"][0].int_value();
	int h = spec["size"][1].int_value();
	Image tex(w, h);

	const auto& cmds = spec["ops"].array_items();
	for (const auto& cmd : cmds) {
		for (const auto& op : s_ops) {
			if (!cmd[op.name].is_null()) {
				const std::string& gen = cmd[op.name].string_value();
				std::cout << "Applying " << gen << " with " << op.name << std::endl;
				break;
			}
		}
	}

	tex.writeTGA(spec["out"].string_value());

	return 0;
}
