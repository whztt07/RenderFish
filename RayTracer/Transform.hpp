#pragma once
#include "Math.hpp"
#include "Ray.hpp"
#include "BBox.hpp"

class Transform
{
public:

	Matrix4x4 m, m_inv;

	static Transform identity;

	Transform() {}
	Transform(const Matrix4x4& mat)
		: m(mat), m_inv(inverse(mat)) {}
	Transform(const Matrix4x4& mat, const Matrix4x4& mat_inv)
		: m(mat), m_inv(mat_inv) {}

	friend Transform inverse(const Transform& t) {
		return Transform(t.m_inv, t.m);
	}

	bool operator==(const Transform& t) const {
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++) {
				if (!equal(m[i][j], t.m[i][j]))
					return false;
			}
		return true;
	}
	bool operator!=(const Transform& t) const { return !operator==(t); }

	Transform operator*(const Transform& t2) const {
		auto m1 = m * t2.m;
		auto m2 = t2.m_inv * m_inv;
		return Transform(m1, m2);
	}
	void operator*=(const Transform& t) {
		m *= t.m;
		m_inv = t.m_inv * m_inv;
	}

#pragma region apply transformation
	inline Vec4 operator()(const Vec4& v4) const {
		return m * v4;
	}
	inline Point operator()(const Point& pt) const {
		auto v = m * Vec4(pt, 1);
		return v.as_point();
	}
	inline void operator()(const Point& pt, Point* transformed_pt) const {
		float x = pt.x, y = pt.y, z = pt.z;	// pt may == *transformed_pt, so save the values before setting new values;
		auto &p = *transformed_pt;
		for (int i = 0; i < 3; i++)
		{
			p[i] = m.m[i][0] * x + m.m[i][1] * y + m.m[i][2] * z + m.m[i][3];
		}
		float w = m.m[3][0] * x + m.m[3][1] * y + m.m[3][2] * z + m.m[3][3];
		if (w != 1.) {
			// REFINE: Point::operator/=() ???
			p.x /= w; p.y /= w; p.z /= w;
		}
	}
	inline Vec3 operator()(const Vec3& v) const {
		// REFINE: p115, avoid Vec3::ctor by not calling operator* ???
		return m * v;
	}
	inline void operator()(const Vec3& v, Vec3* transformed_v) const {
		float x = v.x, y = v.y, z = v.z;
		auto& vv = *transformed_v;
		for (int i = 0; i < 3; i++) {
			vv[i] = m.m[i][0] * x + m.m[i][1] * y + m.m[i][2] * z;
		}
	}
	inline Normal operator()(const Normal& n) const {
		// we do not need to explicitly compute transpose of the inverse of m
		float x = n.x, y = n.y, z = n.z;
		return Normal(
			m_inv.m[0][0] * x + m_inv.m[1][0] * y + m_inv.m[2][0] * z,
			m_inv.m[0][1] * x + m_inv.m[1][1] * y + m_inv.m[2][1] * z,
			m_inv.m[0][2] * x + m_inv.m[1][2] * y + m_inv.m[2][2] * z);
	}
	inline void operator()(const Ray &r, Ray* transformed_r) const {
		(*this)(r.o, &transformed_r->o);
		(*this)(r.d, &transformed_r->d);
	}
	inline Ray operator()(const Ray &r) const {
		Ray ret = r;
		this->operator()(r.o, &ret.o);
		this->operator()(r.d, &ret.d);
		return ret;
	}
	BBox operator()(const BBox& b) const;
#pragma endregion

	bool has_scale() const {
		float lx2 = (*this)(Vec3(1, 0, 0)).length_squared();
		float ly2 = (*this)(Vec3(0, 1, 0)).length_squared();
		float lz2 = (*this)(Vec3(0, 0, 1)).length_squared();
#define NOT_ONE(x) ((x) < .999f || (x) > 1.001f)
		return (NOT_ONE(lx2) || NOT_ONE(ly2) || NOT_ONE(lz2));
#undef NOT_ONE
	}

	bool swaps_handedness() const {
		float det = (
			m.m[0][0] * (m.m[1][1] * m.m[2][2] - m.m[1][2] * m.m[2][1]) -
			m.m[0][1] * (m.m[1][0] * m.m[2][2] - m.m[1][2] * m.m[2][0]) +
			m.m[0][2] * (m.m[1][0] * m.m[2][1] - m.m[1][1] * m.m[2][0]));
		return det < 0.0f;
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

	static Transform look_at(const Point& pos, const Point& look, const Vec3& up);
};
