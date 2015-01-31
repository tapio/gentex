#pragma once

#include "image.hpp"

using namespace glm;

namespace gentex {

// Generators

inline void solidColor(Image& img, const Color& color) {
	img.generate([color](int, int) { return color; });
}





// Filters

inline void grayscaleAverage(Image& img) {
	img.filter([](int, int, Color color) {
		float gray = (color.r + color.g + color.b) / 3.0;
		return Color(gray);
	});
}

inline void grayscaleLuminance(Image& img) {
	img.filter([](int, int, Color color) {
		// http://en.wikipedia.org/wiki/Grayscale
		float gray = 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;
		return Color(gray);
	});
}

} // namespace
