#pragma once
#include <string>
#include <vector>
#include <map>
#include <functional>

#define GLM_FORCE_CXX11
#include <glm/glm.hpp>
#include <glm/gtc/noise.hpp>

#include <json11/json11.hpp>

namespace gentex {

	class Image;
	class Generator;

	typedef unsigned int uint;
	typedef glm::vec3 Color;
	using glm::vec2;
	using glm::vec3;
	using glm::vec4;
	using json11::Json;
	using glm::min;
	using glm::max;
	using glm::exp;
	using glm::pow;
	using glm::clamp;

	typedef std::function<Color(Color, Color)> CompositeFunction;
	typedef std::function<Color(int, int, Color)> FilterFunction;
	typedef std::function<Color(int, int)> GeneratorFunction;

	typedef std::function<void(Image&, CompositeFunction, const Json&, Generator&)> CommandFunction;

	struct Command {
		std::string name;
		CommandFunction cmd;
	};

	struct Op {
		std::string name;
		CompositeFunction op;
	};

	void InitMathParser();

	inline Color saturate(const Color c) { return clamp(c, 0.0f, 1.0f); }

	class Image {
	public:
		Image(int w, int h): w(w), h(h), buffer(w * h) { }
		Image() {}

		Color sample(float u, float v) const {
			return get(u * (w - 1), v * (h - 1));
		}

		Color sampleClamp(float u, float v) const {
			return sample(clamp(u, 0.f, 1.f), clamp(v, 0.f, 1.f));
		}

		Color sampleRepeat(float u, float v) const {
			return get(int(u * (w - 1)) % w, int(v * (h - 1)) % h);
		}

		Color get(int x, int y) const {
			return buffer[x + y * w];
		}

		Color getClamp(int x, int y) const {
			return get(clamp(x, 0, w - 1), clamp(y, 0, h - 1));
		}

		Color getRepeat(int x, int y) const {
			return get(x % w, y % h);
		}

		void generate(GeneratorFunction func) {
			for (int y = 0; y < h; ++y) {
				for (int x = 0; x < w; ++x) {
					buffer[y * w + x] = func(x, y);
				}
			}
		}

		void composite(GeneratorFunction func, CompositeFunction op) {
			for (int y = 0; y < h; ++y) {
				for (int x = 0; x < w; ++x) {
					Color& color = buffer[y * w + x];
					color = op(color, func(x, y));
				}
			}
		}

		void filter(FilterFunction func, CompositeFunction op) {
			for (int y = 0; y < h; ++y) {
				for (int x = 0; x < w; ++x) {
					Color& color = buffer[y * w + x];
					color = op(color, func(x, y, color));
				}
			}
		}

		void write(const std::string& filepath = "out.png") const;
		void writeTGA(const std::string& filepath = "out.tga", bool rleCompress = false) const;
		void writePNG(const std::string& filepath = "out.png") const;
		void writeJPG(const std::string& filepath = "out.jpg", int quality = 95) const;
		const std::vector<char> getBytes() const;

		int w = 0, h = 0;
		static const int channels = 3; // TODO: Support different channel count, i.e. alpha?
		std::vector<Color> buffer;
	};

	class Generator {
	public:
		Generator(int width, int height): image(width, height) { }

		void processCommand(const Json& cmd);

		Image image;
		std::map<std::string, Image> namedImages;
	};

} // namespace
