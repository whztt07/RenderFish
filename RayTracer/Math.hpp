#ifndef RTMATH_H
#define RTMATH_H
#include "RenderFish.hpp"
#include <iostream>
#include <cassert>
#include <cmath>
#include <cstdint>

#include "MathHelper.hpp"
#include "Vector.hpp"
#include "Matrix4x4.hpp"

// note: Vector in RenderFish is COLUMN vector
// Matrix4x4 in RenderFish is ROW matrix
// matrix-vector multiplication: mat * vec;

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

static float distance(const Point &p1, const Point &p2)
{ 
	return (p1 - p2).length(); 
}

static float distance_squared(const Point &p1, const Point &p2) 
{
	return (p1 - p2).length_squared();
}

static Point center(const Point& p1, const Point& p2)
{
	return Point((p1.x + p2.x) * 0.5f, (p1.y + p2.y) * 0.5f, (p1.z + p2.z) * 0.5f);
}

// in and normal are normalized
static Vec3 reflect_normalized(const Vec3& in, const Vec3& normal)
{
	//return (n * 2 * (dot(n, v)) - v).normalize();
	return (in - 2 * dot(in, normal) * normal).normalize();
}

static bool perpendicular(const Vec3& u, const Vec3& v)
{
	return zero(dot(u, v));
}

float determinant(const Matrix4x4& rhs);

Matrix4x4 inverse(const Matrix4x4& rhs);

inline Matrix4x4 transpose(const Matrix4x4& m)
{
	return Matrix4x4::transpose(m);
}

#endif