#pragma once
#include <string>
#include <vector>
#include <functional>

#define GLM_FORCE_CXX11
#include <glm/glm.hpp>
#include <glm/gtc/noise.hpp>

#include <json11/json11.hpp>

namespace gentex {

	class Image;

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

	typedef std::function<Color(Color, Color)> CompositeFunction;
	typedef std::function<Color(int, int, Color)> FilterFunction;
	typedef std::function<Color(int, int)> GeneratorFunction;

	typedef std::function<void(Image&, CompositeFunction, const Json&)> CommandFunction;

	struct Command {
		std::string name;
		CommandFunction cmd;
	};

	struct Op {
		std::string name;
		CompositeFunction op;
	};

	void initMathParser();

	inline Color saturate(const Color c) { return clamp(c, 0.0f, 1.0f); }

	CommandFunction& getCommand(const std::string& name);


	class Image {
	public:
		Image(int w, int h): w(w), h(h), buffer(w * h) { }

		Color sample(float u, float v) const {
			return get(u / w, v / h);
		}

		Color sampleClamp(float u, float v) const {
			u = u < 0 ? 0 : (u > 1.0 ? 1.0 : u);
			v = v < 0 ? 0 : (v > 1.0 ? 1.0 : v);
			return get(u / w, v / h);
		}

		Color sampleRepeat(float u, float v) const {
			return get(int(u / w) % w, int(v / h) % h);
		}

		Color get(int x, int y) const {
			return buffer[x + y * w];
		}

		Color getClamp(int x, int y) const {
			x = x < 0 ? 0 : (x > w ? w : x);
			y = y < 0 ? 0 : (y > h ? h : y);
			return get(x, y);
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

		int w, h;
		static const int channels = 3; // TODO: Support different channel count, i.e. alpha?
		std::vector<Color> buffer;
	};

} // namespace
