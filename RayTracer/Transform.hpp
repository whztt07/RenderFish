#pragma once
#include "Math.hpp"

class Transform
{
public:

	Matrix4x4 m, m_inv;

	Transform() {}
	Transform(const Matrix4x4& mat)
		: m(mat), m_inv(inverse(mat)) {}
	Transform(const Matrix4x4& mat, const Matrix4x4& mat_inv)
		: m(mat), m_inv(mat_inv) {}

	friend Transform inverse(const Transform& t) {
		return Transform(t.m_inv, t.m);
	}

	bool operator==(const Transform& t) const {
		// TODO
	}

	Vec4 operator()(const Vec4& v4) const {
		return m * v4;
	}
	Point operator()(const Point& p) const {
		auto v = m * Vec4(p, 1);
		return Point(v.x, v.y, v.z);
	}
	Vec3 operator()(const Vec3& v) const {
		return (m * Vec4(v, 0)).xyz();
	}

	bool has_scale() const {
		float lx2 = (*this)(Vec3(1, 0, 0)).length_squared();
		float ly2 = (*this)(Vec3(0, 1, 0)).length_squared();
		float lz2 = (*this)(Vec3(0, 0, 1)).length_squared();
#define NOT_ONE(x) ((x) < .999f || (x) > 1.001f)
		return (NOT_ONE(lx2) || NOT_ONE(ly2) || NOT_ONE(lz2));
#undef NOT_ONE
	}

	static Transform translate(float x, float y, float z) {
		Matrix4x4 m(
			1, 0, 0, x,
			0, 1, 0, y,
			0, 0, 1, z,
			0, 0, 0, 1);
		Matrix4x4 minv(
			1, 0, 0, -x,
			0, 1, 0, -y,
			0, 0, 1, -z,
			0, 0, 0, 1
			);
		return Transform(m, minv);
	}

	static Transform translate(const Vec3& delta) {
		return translate(delta.x, delta.y, delta.z);
	}

	static Transform scale(float x, float y, float z) {
		Matrix4x4 m(
			x, 0, 0, 0,
			0, y, 0, 0,
			0, 0, z, 0,
			0, 0, 0, 1);
		Matrix4x4 minv(
			1.f/x,	    0,		0,	0,
			0,		1.f/y,		0,	0,
			0,		    0,	1.f/z,	0,
			0,		    0,		0,	1
			);
		return Transform(m, minv);
	}

	static Transform rotate_x(float degrees);
	static Transform rotate_y(float degrees);
	static Transform rotate_z(float degrees);
	static Transform rotate(float degrees, const Vec3 &axis);
};

