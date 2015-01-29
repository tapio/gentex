
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cctype>

std::string readFile(const std::string& path) {
	std::ifstream f(path);
	return std::string(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
}

void eatWhitespace(char* &ptr) {
	while (isspace(*ptr)) ++ptr;
}

std::string readToken(char* &ptr) {
	std::string token;
	while (isalnum(*ptr)) { token += *ptr; ++ptr; };
	ptr++;
	return token;
}

int main(int argc, char** argv) {
	std::string source = readFile(argv[1]);
	char* ptr = &source[0];
	while (*ptr) {
		std::cout << "before eat " << (ptr - &source[0]) << std::endl;
		eatWhitespace(ptr);
		std::cout << "before token " << (ptr - &source[0]) << std::endl;
		std::cout << readToken(ptr) << std::endl;
	}

	return 0;
}
