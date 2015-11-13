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

	//Transform& operator=(const Transform& t) {
	//	for (int i = 0; i < 4; ++i)
	//		for (int j = 0; j < 4; ++j) {
	//			m[i][j] = t.m[i][j];
	//			m_inv[i][j] = t.m_inv[i][j];
	//		}
	//	return *this;
	//}

	friend Transform inverse(const Transform& t) {
		return Transform(t.m_inv, t.m);
	}
	friend Transform transpose(const Transform &t) {
		return Transform(transpose(t.m), transpose(t.m_inv));
	}

	bool operator==(const Transform& t) const {
		return t.m == m && t.m_inv == m_inv;
	}
	bool operator!=(const Transform& t) const { 
		return !operator==(t);
	}

	Transform operator*(const Transform& t2) const {
		auto m1 = m * t2.m;
		auto m2 = t2.m_inv * m_inv;
		return Transform(m1, m2);
	}
	Transform& operator*=(const Transform& t) {
		m *= t.m;
		m_inv = t.m_inv * m_inv;
		return *this;
	}

	inline Vec4 operator()(const Vec4& v4) const;
	inline Point operator()(const Point& pt) const;
	inline void operator()(const Point& pt, Point* transformed_pt) const;
	inline Vec3 operator()(const Vec3& v) const;
	inline void operator()(const Vec3& v, Vec3* transformed_v) const;
	inline Normal operator()(const Normal& n) const;
	inline void Transform::operator()(const Normal &n, Normal *nt) const;
	inline Ray operator()(const Ray &r) const;
	inline void operator()(const Ray &r, Ray* transformed_r) const;
	inline RayDifferential operator()(const RayDifferential &r) const;
	inline void operator()(const RayDifferential &r, RayDifferential *rt) const;
	BBox operator()(const BBox& b) const;

	bool has_scale() const;

	bool swaps_handedness() const;
};

Transform translate(float x, float y, float z);
Transform translate(const Vec3& delta);
Transform scale(float x, float y, float z);
Transform rotate_x(float degrees);
Transform rotate_y(float degrees);
Transform rotate_z(float degrees);
Transform rotate(float degrees, const Vec3 &axis);
Transform look_at(const Point& pos, const Point& look, const Vec3& up = Vec3::axis_y);
Transform orthographic(float z_near, float z_far);
Transform perspective(float fov, float z_near, float z_far);

inline Vec4 Transform::operator()(const Vec4& v4) const
{
	return m * v4;
}

inline Point Transform::operator()(const Point& pt) const
{
	auto v = m * Vec4(pt, 1);
	return v.as_point();
}

inline void Transform::operator()(const Point& pt, Point* transformed_pt) const
{
	float x = pt.x, y = pt.y, z = pt.z;	// pt may == *transformed_pt, so save the values before setting new values;
	auto &p = *transformed_pt;
	for (int i = 0; i < 3; i++) {
		p[i] = m.m[i][0] * x + m.m[i][1] * y + m.m[i][2] * z + m.m[i][3];
	}
	float w = m.m[3][0] * x + m.m[3][1] * y + m.m[3][2] * z + m.m[3][3];
	Assert(w != 0);
	if (w != 1.) {
		// REFINE: Point::operator/=() ???
		p.x /= w; p.y /= w; p.z /= w;
	}
}

inline Vec3 Transform::operator()(const Vec3& v) const
{
	// REFINE: p115, avoid Vec3::ctor by not calling operator* ???
	return m * v;
}

inline void Transform::operator()(const Vec3& v, Vec3* transformed_v) const
{
	float x = v.x, y = v.y, z = v.z;
	auto& vv = *transformed_v;
	for (int i = 0; i < 3; i++) {
		vv[i] = m.m[i][0] * x + m.m[i][1] * y + m.m[i][2] * z;
	}
}

inline Normal Transform::operator()(const Normal& n) const
{
	// we do not need to explicitly compute transpose of the inverse of m
	float x = n.x, y = n.y, z = n.z;
	return Normal(
		m_inv.m[0][0] * x + m_inv.m[1][0] * y + m_inv.m[2][0] * z,
		m_inv.m[0][1] * x + m_inv.m[1][1] * y + m_inv.m[2][1] * z,
		m_inv.m[0][2] * x + m_inv.m[1][2] * y + m_inv.m[2][2] * z);
}

inline void Transform::operator()(const Normal &n, Normal *nt) const
{
	float x = n.x, y = n.y, z = n.z;
	nt->x = m_inv.m[0][0] * x + m_inv.m[1][0] * y + m_inv.m[2][0] * z;
	nt->y = m_inv.m[0][1] * x + m_inv.m[1][1] * y + m_inv.m[2][1] * z;
	nt->z = m_inv.m[0][2] * x + m_inv.m[1][2] * y + m_inv.m[2][2] * z;
}

inline Ray Transform::operator()(const Ray &r) const
{
	Ray ret = r;
	this->operator()(r.o, &ret.o);
	this->operator()(r.d, &ret.d);
	return ret;
}

inline void Transform::operator()(const Ray &r, Ray* transformed_r) const
{
	if (&r != transformed_r) {
		transformed_r->mint = r.mint;
		transformed_r->maxt = r.maxt;
		transformed_r->time = r.time;
		transformed_r->depth = r.depth;
	}
	(*this)(r.o, &transformed_r->o);
	(*this)(r.d, &transformed_r->d);
}

inline RayDifferential Transform::operator()(const RayDifferential &r) const
{
	RayDifferential ret = r;
	//(*this)(Ray(r), (Ray*)&ret);
	(*this)(r.o, &ret.o);
	(*this)(r.d, &ret.d);
	(*this)(r.rx_origin, &ret.rx_origin);
	(*this)(r.ry_origin, &ret.ry_origin);
	(*this)(r.rx_direction, &ret.rx_direction);
	(*this)(r.ry_direction, &ret.ry_direction);
	//ret.has_differentials = r.has_differentials;
	return ret;
}

inline void Transform::operator()(const RayDifferential &r, RayDifferential *rt) const
{
	(*this)(Ray(r), (Ray*)rt);
	rt->has_differentials = r.has_differentials;
	(*this)(r.rx_origin, &rt->rx_origin);
	(*this)(r.ry_origin, &rt->ry_origin);
	(*this)(r.rx_direction, &rt->rx_direction);
	(*this)(r.ry_direction, &rt->ry_direction);
}