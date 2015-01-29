#include "gentex.hpp"

using namespace gentex;

int main(int argc, char** argv) {
	Image img(64, 64);
	solidColor(img, vec4(0.0, 0.5, 0.0, 1.0));
	img.writeTGA("solid-green.tga");

	return 0;
}
