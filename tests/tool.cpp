
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cctype>

#include "gentex.hpp"
#include "json11/json11.hpp"

using namespace gentex;
using namespace json11;

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
	solidColor(tex, vec3(0.0, 0.2, 0.3));
	tex.writeTGA(spec["out"].string_value());

	return 0;
}
