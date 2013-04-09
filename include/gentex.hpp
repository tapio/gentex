#pragma once

#include "image.hpp"

using namespace glm;

namespace gentex {

// Generators

inline void solidColor(Image& img, const vec3& color) {
	img.generate([color](int, int) { return color; });
}





// Filters

inline void grayscaleAverage(Image& img) {
	img.filter([](int, int, vec3 color) {
		float gray = (color.r + color.g + color.b) / 3.0;
		return vec3(gray);
	});
}

inline void grayscaleLuminance(Image& img) {
	img.filter([](int, int, vec3 color) {
		// http://en.wikipedia.org/wiki/Grayscale
		float gray = 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;
		return vec3(gray);
	});
}

} // namespace
