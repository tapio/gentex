#pragma once
#include <cmath>

namespace gentex {

static constexpr float PI = 3.1415926535f;
static constexpr float TWOPI = 2 * PI;
//static constexpr float PI2 = PI * 0.5f;
//static constexpr float PI4 = PI * 0.25f;
static constexpr float DEG_TO_RAD = PI / 180.f;
static constexpr float RAD_TO_DEG = 180.f / PI;

typedef unsigned int uint;

template<typename T>
constexpr inline T abs(T a) { return a < 0 ? -a : a; }
template<typename T>
constexpr inline T min(T a, T b) { return a < b ? a : b; }
template<typename T>
constexpr inline T max(T a, T b) { return a > b ? a : b; }
template<typename T>
constexpr inline T clamp(T x, T lo, T hi) { return x < lo ? lo : (x > hi ? hi : x); }
constexpr inline float saturate(float x) { return clamp(x, 0.f, 1.f); }
constexpr inline float mix(float a, float b, float t) { return a + (b - a) * t; }
constexpr inline float sign(float x) { return x > 0.f ? 1.f : (x < 0.f ? -1.f : 0.f); }

struct vec2
{
	union {
		struct { float x, y; };
		struct { float r, g; };
		struct { float s, t; };
		float v[2];
	};

	constexpr vec2(float v = 0.f): x(v), y(v) {}
	constexpr vec2(float x_, float y_): x(x_), y(y_) {}

	constexpr vec2 operator+(const vec2& rhs) const { return { x + rhs.x, y + rhs.y }; }
	constexpr vec2 operator-(const vec2& rhs) const { return { x - rhs.x, y - rhs.y }; }
	constexpr vec2 operator*(const vec2& rhs) const { return { x * rhs.x, y * rhs.y }; }
	constexpr vec2 operator/(const vec2& rhs) const { return { x / rhs.x, y / rhs.y }; }
	constexpr vec2 operator*(float rhs) const { return { x * rhs, y * rhs }; }
	constexpr vec2 operator/(float rhs) const { return { x / rhs, y / rhs }; }

	vec2& operator+=(const vec2& rhs) { x += rhs.x; y += rhs.y; return *this; }
	vec2& operator-=(const vec2& rhs) { x -= rhs.x; y -= rhs.y; return *this; }
	vec2& operator*=(const vec2& rhs) { x *= rhs.x; y *= rhs.y; return *this; }
	vec2& operator/=(const vec2& rhs) { x /= rhs.x; y /= rhs.y; return *this; }
	vec2& operator*=(float rhs) { x *= rhs; y *= rhs; return *this; }
	vec2& operator/=(float rhs) { x /= rhs; y /= rhs; return *this; }

	constexpr bool operator==(const vec2& rhs) const { return x == rhs.x && y == rhs.y; }
	constexpr bool operator!=(const vec2& rhs) const { return !(*this == rhs); }

	vec2& operator-() { x = -x; y = -y; return *this; }
	constexpr float operator[](int index) { return v[index]; }
};

struct vec3
{
	union {
		struct { float x, y, z; };
		struct { float r, g, b; };
		struct { float s, t, p; };
		float v[3];
	};

	constexpr vec3(float v = 0.f): x(v), y(v), z(v) {}
	constexpr vec3(float x_, float y_, float z_): x(x_), y(y_), z(z_) {}

	constexpr vec3 operator+(const vec3& rhs) const { return { x + rhs.x, y + rhs.y, z + rhs.z }; }
	constexpr vec3 operator-(const vec3& rhs) const { return { x - rhs.x, y - rhs.y, z - rhs.z }; }
	constexpr vec3 operator*(const vec3& rhs) const { return { x * rhs.x, y * rhs.y, z * rhs.z }; }
	constexpr vec3 operator/(const vec3& rhs) const { return { x / rhs.x, y / rhs.y, z / rhs.z }; }
	constexpr vec3 operator*(float rhs) const { return { x * rhs, y * rhs, z * rhs }; }
	constexpr vec3 operator/(float rhs) const { return { x / rhs, y / rhs, z / rhs }; }

	vec3& operator+=(const vec3& rhs) { x += rhs.x; y += rhs.y; z += rhs.z; return *this; }
	vec3& operator-=(const vec3& rhs) { x -= rhs.x; y -= rhs.y; z -= rhs.z; return *this; }
	vec3& operator*=(const vec3& rhs) { x *= rhs.x; y *= rhs.y; z *= rhs.z; return *this; }
	vec3& operator/=(const vec3& rhs) { x /= rhs.x; y /= rhs.y; z /= rhs.z; return *this; }
	vec3& operator*=(float rhs) { x *= rhs; y *= rhs; z *= rhs; return *this; }
	vec3& operator/=(float rhs) { x /= rhs; y /= rhs; z /= rhs; return *this; }

	constexpr bool operator==(const vec3& rhs) const { return x == rhs.x && y == rhs.y && z == rhs.z; }
	constexpr bool operator!=(const vec3& rhs) const { return !(*this == rhs); }

	vec3& operator-() { x = -x; y = -y; z = -z; return *this; }
	constexpr float operator[](int index) { return v[index]; }
};

struct vec4
{
	union {
		struct { float x, y, z, w; };
		struct { float r, g, b, a; };
		struct { float s, t, p, q; };
		float v[4];
	};

	constexpr vec4(float v = 0.f) : x(v), y(v), z(v), w(v) {}
	constexpr vec4(float x_, float y_, float z_, float w_) : x(x_), y(y_), z(z_), w(w_) {}

	constexpr vec4 operator+(const vec4& rhs) const { return { x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w }; }
	constexpr vec4 operator-(const vec4& rhs) const { return { x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w }; }
	constexpr vec4 operator*(const vec4& rhs) const { return { x * rhs.x, y * rhs.y, z * rhs.z, w * rhs.w }; }
	constexpr vec4 operator/(const vec4& rhs) const { return { x / rhs.x, y / rhs.y, z / rhs.z, w / rhs.w }; }
	constexpr vec4 operator*(float rhs) const { return { x * rhs, y * rhs, z * rhs, w * rhs }; }
	constexpr vec4 operator/(float rhs) const { return { x / rhs, y / rhs, z / rhs, w / rhs }; }

	vec4& operator+=(const vec4& rhs) { x += rhs.x; y += rhs.y; z += rhs.z; w += rhs.w; return *this; }
	vec4& operator-=(const vec4& rhs) { x -= rhs.x; y -= rhs.y; z -= rhs.z; w -= rhs.w; return *this; }
	vec4& operator*=(const vec4& rhs) { x *= rhs.x; y *= rhs.y; z *= rhs.z; w *= rhs.w; return *this; }
	vec4& operator/=(const vec4& rhs) { x /= rhs.x; y /= rhs.y; z /= rhs.z; w /= rhs.w; return *this; }
	vec4& operator*=(float rhs) { x *= rhs; y *= rhs; z *= rhs; w *= rhs; return *this; }
	vec4& operator/=(float rhs) { x /= rhs; y /= rhs; z /= rhs; w /= rhs; return *this; }

	constexpr bool operator==(const vec4& rhs) const { return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w; }
	constexpr bool operator!=(const vec4& rhs) const { return !(*this == rhs); }

	vec4& operator-() { x = -x; y = -y; z = -z; w = -w; return *this; }
	constexpr float operator[](int index) { return v[index]; }
};

inline constexpr float dot(const vec2& lhs, const vec2& rhs) { return lhs.x * rhs.x + lhs.y * rhs.y; }
inline constexpr float length2(const vec2& v) { return dot(v, v); }
inline           float length(const vec2& v) { return std::sqrt(length2(v)); }
inline constexpr float distance2(const vec2& lhs, const vec2& rhs) { return length2(rhs - lhs); }
inline           float distance(const vec2& lhs, const vec2& rhs) { return length(rhs - lhs); }
inline           vec2 normalize(const vec2& v) { float l = 1.f / length(v); return v * l; }
inline constexpr vec2 abs(const vec2& v) { return { abs(v.x), abs(v.y) }; }
inline constexpr vec2 min(const vec2& lhs, const vec2& rhs) { return { min(lhs.x, rhs.x), min(lhs.y, rhs.y) }; }
inline constexpr vec2 max(const vec2& lhs, const vec2& rhs) { return { max(lhs.x, rhs.x), max(lhs.y, rhs.y) }; }
inline constexpr float min(const vec2& v) { return min(v.x, v.y); }
inline constexpr float max(const vec2& v) { return max(v.x, v.y); }
inline constexpr vec2 clamp(const vec2& v, const vec2& lo, const vec2& hi) { return { clamp(v.x, lo.x, hi.x), clamp(v.y, lo.y, hi.y) }; }
inline constexpr vec2 mix(const vec2& a, const vec2& b, float t) { return a + (b - a) * t; }

inline constexpr vec3 cross(const vec3& lhs, const vec3& rhs) { return { lhs.y * rhs.z - lhs.z * rhs.y, lhs.z * rhs.x - lhs.x * rhs.z, lhs.x * rhs.y - lhs.y * rhs.x }; }
inline constexpr float dot(const vec3& lhs, const vec3& rhs) { return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z; }
inline constexpr float length2(const vec3& v) { return dot(v, v); }
inline           float length(const vec3& v) { return std::sqrt(length2(v)); }
inline constexpr float distance2(const vec3& lhs, const vec3& rhs) { return length2(rhs - lhs); }
inline           float distance(const vec3& lhs, const vec3& rhs) { return length(rhs - lhs); }
inline           vec3 normalize(const vec3& v) { float l = 1.f / length(v); return v * l; }
inline constexpr vec3 abs(const vec3& v) { return { abs(v.x), abs(v.y), abs(v.z) }; }
inline constexpr vec3 min(const vec3& lhs, const vec3& rhs) { return { min(lhs.x, rhs.x), min(lhs.y, rhs.y), min(lhs.z, rhs.z) }; }
inline constexpr vec3 max(const vec3& lhs, const vec3& rhs) { return { max(lhs.x, rhs.x), max(lhs.y, rhs.y), max(lhs.z, rhs.z) }; }
inline constexpr float min(const vec3& v) { return min(v.x, min(v.y, v.z)); }
inline constexpr float max(const vec3& v) { return max(v.x, max(v.y, v.z)); }
inline constexpr vec3 clamp(const vec3& v, const vec3& lo, const vec3& hi) { return { clamp(v.x, lo.x, hi.x), clamp(v.y, lo.y, hi.y), clamp(v.z, lo.z, hi.z) }; }
inline constexpr vec3 mix(const vec3& a, const vec3& b, float t) { return a + (b - a) * t; }
inline           vec3 triangleNormal(const vec3& a, const vec3& b, const vec3& c) { return normalize(cross(b - a, c - a)); }

inline constexpr float dot(const vec4& lhs, const vec4& rhs) { return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z + lhs.w * rhs.w; }
inline constexpr float length2(const vec4& v) { return dot(v, v); }
inline           float length(const vec4& v) { return std::sqrt(length2(v)); }
inline constexpr float distance2(const vec4& lhs, const vec4& rhs) { return length2(rhs - lhs); }
inline           float distance(const vec4& lhs, const vec4& rhs) { return length(rhs - lhs); }
inline           vec4 normalize(const vec4& v) { float l = 1.f / length(v); return v * l; }
inline constexpr vec4 abs(const vec4& v) { return { abs(v.x), abs(v.y), abs(v.z), abs(v.w) }; }
inline constexpr vec4 min(const vec4& lhs, const vec4& rhs) { return { min(lhs.x, rhs.x), min(lhs.y, rhs.y), min(lhs.z, rhs.z), min(lhs.w, rhs.w) }; }
inline constexpr vec4 max(const vec4& lhs, const vec4& rhs) { return { max(lhs.x, rhs.x), max(lhs.y, rhs.y), max(lhs.z, rhs.z), max(lhs.w, rhs.w) }; }
inline constexpr float min(const vec4& v) { return min(v.x, min(v.y, min(v.z, v.w))); }
inline constexpr float max(const vec4& v) { return max(v.x, max(v.y, max(v.z, v.w))); }
inline constexpr vec4 clamp(const vec4& v, const vec4& lo, const vec4& hi) { return { clamp(v.x, lo.x, hi.x), clamp(v.y, lo.y, hi.y), clamp(v.z, lo.z, hi.z), clamp(v.w, lo.w, hi.w) }; }
inline constexpr vec4 mix(const vec4& a, const vec4& b, float t) { return a + (b - a) * t; }

} // namespace gentex
