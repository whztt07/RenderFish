#ifndef RTMATH_H
#define RTMATH_H
#include <iostream>
#include <cassert>
#include <cmath>

#define EPSILON 1e-6f;
#define ZERO(x) abs(x) < EPSILON
const float PI = 3.1415926536f;
const float INV_PI = 1.0f / PI;
const float TO_DEGREES = 180.0f * INV_PI;
const float TO_RADIANS = 1.0f / TO_DEGREES;

class Vec2 {
public:
	float x, y;
};

template < class T, class U >
static float dot(const T& lhs, const U& rhs)
{
	return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}

class Vec3 {
public:
	union {
		struct { float x, y, z; };
		float m[3];
	};
	
	Vec3() : Vec3(0, 0, 0) {}
	Vec3(float x) : Vec3(x, x, x) {}
	Vec3(const Vec3& v) : Vec3(v.x, v.y, v.z) {}
	Vec3(float x, float y, float z) : x(x), y(y), z(z) { assert(!has_NaNs()); }

	bool has_NaNs() const { return isnan(x) || isnan(y) || isnan(z); }

	friend std::ostream& operator << (std::ostream& os, const Vec3& v)
	{
		os << v.x << ", " << v.y << ", " << v.z;
		return os;
	}

	float length_squared() { return x*x + y*y + z*z; }
	float length() const { return sqrtf(x*x + y*y + z*z); }
	const Vec3 normalize() const { float l = length(); return Vec3(x / l, y / l, z / l); }
	void normalize_self() { float l = length(); x /= l, y /= l; z /= l; }

	float& operator[](const int index) { return m[index]; }
	float operator[](const int index) const { return m[index]; }
	Vec3& operator=(const Vec3& v) { x = v.x; y = v.y; z = v.z; return *this; }
	Vec3 operator-() const { return Vec3(-x, -y, -z); }
	Vec3 operator*(const float f) const { return Vec3(x * f, y * f, z * f); }
	Vec3 operator/(const float f) const { return Vec3(x / f, y / f, z / f); }
	friend Vec3 operator*(const float f, const Vec3& v) { return Vec3(v.x * f, v.y * f, v.z * f); }
	friend Vec3 operator/(const float f, const Vec3& v) { return Vec3(v.x / f, v.y / f, v.z / f); }
	Vec3 operator+(const Vec3& v) const { return Vec3(x + v.x, y + v.y, z + v.z); }
	Vec3 operator-(const Vec3& v) const { return Vec3(x - v.x, y - v.y, z - v.z); }
	Vec3 operator+=(const Vec3& v) { x += v.x; y += v.y; z += v.z; }
	Vec3 operator-=(const Vec3& v) { x -= v.x; y -= v.y; z -= v.z; }
	void operator*=(const float f) { x *= f; y *= f; z *= f; }
	void operator/=(const float f) { x /= f; y /= f; z /= f; }
};

class Point
{
public:
	float x, y, z;

	Point() : Point(0, 0, 0) {}
	Point(float x, float y, float z) : x(x), y(y), z(z) {}

	Point operator+(const Vec3 &v) const { return Point(x + v.x, y + v.y, z + v.z); }
	Point operator-(const Vec3 &v) const { return Point(x - v.x, y - v.y, z - v.z); }
	Vec3 operator-(const Point &v) const { return Vec3(x - v.x, y - v.y, z - v.z); }
	Point& operator+=(const Vec3 &v) { x += v.x; y += v.y; z += v.z; return *this; }
	Point& operator-=(const Vec3 &v) { x -= v.x; y -= v.y; z -= v.z; return *this; }

	static float distance(const Point &p1, const Point &p2) { return (p1 - p2).length(); }
	static float distance_squared(const Point &p1, const Point &p2) { return (p1 - p2).length_squared(); }
};

class Normal
{
public:
	float x, y, z;

	Normal() : Normal(0, 0, 0) {}
	explicit Normal(float x, float y, float z) : x(x), y(y), z(z) {}
	//explicit Normal(const Vec3& v) : x(v.x), y(v.y), z(v.z) {}
	//explicit Normal(const Normal& n) : x(n.x), y(n.y), z(n.z) {}

	Normal& operator=(const Normal& v) { x = v.x; y = v.y; z = v.z; return *this; }
	Normal operator-() const { return Normal(-x, -y, -z); }
	Normal operator*(const float f) const { return Normal(x * f, y * f, z * f); }
	Normal operator/(const float f) const { return Normal(x / f, y / f, z / f); }
	friend Normal operator*(const float f, const Normal& v) { return Normal(v.x * f, v.y * f, v.z * f); }
	friend Normal operator/(const float f, const Normal& v) { return Normal(v.x / f, v.y / f, v.z / f); }
	Normal operator+(const Normal& v) const { return Normal(x + v.x, y + v.y, z + v.z); }
	Normal operator-(const Normal& v) const { return Normal(x - v.x, y - v.y, z - v.z); }
	Normal operator+=(const Normal& v) { x += v.x; y += v.y; z += v.z; }
	Normal operator-=(const Normal& v) { x -= v.x; y -= v.y; z -= v.z; }
	void operator*=(const float f) { x *= f; y *= f; z *= f; }
	void operator/=(const float f) { x /= f; y /= f; z /= f; }

	static Normal face_forward(const Normal& n, const Vec3& v) { return (dot(n, v) < 0.f) ? -n : n; }
};
 
class Vec4 {
public:
	float x, y, z;
};

static float dot(const Vec3& u, const Vec3& v)
{
	return u.x * v.x + u.y * v.y + u.z * v.z;
}

static Vec3 cross(const Vec3& u, const Vec3& v)
{
	return Vec3(u.y * v.z - u.z * v.y, u.z * v.x - u.x * v.z, u.x * v.y - u.y * v.x);
}

static Vec3 normalize(const Vec3& v)
{
	//float l = v.length();
	return v.normalize();
}

// in and normal are normalized
static Vec3 reflect_normalized(const Vec3& in, const Vec3& normal)
{
	//return (n * 2 * (dot(n, v)) - v).normalize();
	return (in - 2 * dot(in, normal) * normal).normalize();
}

static float clamp(float f, float min = 0, float max = 1.0f)
{
	f = f < max ? f : max;
	return f > min ? f : min;
}

static float to_degrees(float radians)
{
	return radians * TO_DEGREES;
}

static float to_radians(float degrees)
{
	return degrees * TO_RADIANS;
}

static bool perpendicular(const Vec3& u, const Vec3& v)
{
	return ZERO(dot(u, v));
}
#endif