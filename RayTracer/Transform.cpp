#include "Transform.hpp"

Transform Transform::identity = Transform(Matrix4x4(), Matrix4x4());

BBox Transform::operator()(const BBox& b) const {
#if 0
	const Transform &M = *this;
	BBox ret(M(Point(b.pmin.x, b.pmin.y, b.pmin.z)));
	ret = combine(ret, M(Point(b.pmax.x, b.pmin.y, b.pmin.z)));
	ret = combine(ret, M(Point(b.pmin.x, b.pmax.y, b.pmin.z)));
	ret = combine(ret, M(Point(b.pmin.x, b.pmin.y, b.pmax.z)));
	ret = combine(ret, M(Point(b.pmin.x, b.pmax.y, b.pmax.z)));
	ret = combine(ret, M(Point(b.pmax.x, b.pmax.y, b.pmin.z)));
	ret = combine(ret, M(Point(b.pmax.x, b.pmin.y, b.pmax.z)));
	ret = combine(ret, M(Point(b.pmax.x, b.pmax.y, b.pmax.z)));
	return ret;
#else
	// note the fact that the eight corner points are linear combinations 
	//	of three axis-aligned basis vectors and a single corner point
	// PBRT p104 exercise 2.1
	const Transform & T = *this;
	auto d = b.pmax - b.pmin;
	auto x = T(Vec3(d.x, 0, 0));
	auto y = T(Vec3(0, d.y, 0));
	auto z = T(Vec3(0, 0, d.z));
	auto pmin = T(b.pmin);
	auto pmax = pmin;
#define NEGTIVE(v) ((v) < 0 ? (v) : 0)
#define POSITIVE(v) ((v) > 0 ? (v) : 0)
	pmin.x += NEGTIVE(x.x) + NEGTIVE(y.x) + NEGTIVE(z.x);
	pmin.y += NEGTIVE(x.y) + NEGTIVE(y.y) + NEGTIVE(z.y);
	pmin.z += NEGTIVE(x.z) + NEGTIVE(y.z) + NEGTIVE(z.z);
	pmax.x += POSITIVE(x.x) + POSITIVE(y.x) + POSITIVE(z.x);
	pmax.y += POSITIVE(x.y) + POSITIVE(y.y) + POSITIVE(z.y);
	pmax.z += POSITIVE(x.z) + POSITIVE(y.z) + POSITIVE(z.z);
#undef NEGTIVE
#undef POSITIVE
	//return BBox(pmin, pmin + x + y + z);
	return BBox(pmin, pmax);
#endif
}

Transform Transform::rotate_x(float degrees)
{
	float sin_t = sinf(radians(degrees));
	float cos_t = cosf(radians(degrees));
	Matrix4x4 m(
		1, 0, 0, 0,
		0, cos_t, -sin_t, 0,
		0, sin_t, cos_t, 0,
		0, 0, 0, 1);
	return Transform(m, transpose(m));
}

Transform Transform::rotate_y(float degrees)
{
	float sin_t = sinf(radians(degrees));
	float cos_t = cosf(radians(degrees));
	Matrix4x4 m(
		cos_t, 0, sin_t, 0,
		0, 1, 0, 0,
		-sin_t, 0, cos_t, 0,
		0, 0, 0, 1);
	return Transform(m, transpose(m));
}

Transform Transform::rotate_z(float degrees)
{
	float sin_t = sinf(radians(degrees));
	float cos_t = cosf(radians(degrees));
	Matrix4x4 m(
		cos_t, -sin_t, 0, 0,
		sin_t, cos_t, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1);
	return Transform(m, transpose(m));
}

Transform Transform::rotate(float degrees, const Vec3 &axis)
{
	Vec3 a = normalize(axis);
	float s = sinf(radians(degrees));
	float c = cosf(radians(degrees));
	float m[4][4];

	m[0][0] = a.x * a.x + (1.f - a.x * a.x) * c;
	m[0][1] = a.x * a.y * (1.f - c) - a.z * s;
	m[0][2] = a.x * a.z * (1.f - c) + a.y * s;
	m[0][3] = 0;

	m[1][0] = a.x * a.y * (1.f - c) + a.z * s;
	m[1][1] = a.y * a.y + (1.f - a.y * a.y) * c;
	m[1][2] = a.y * a.z * (1.f - c) - a.x * s;
	m[1][3] = 0;

	m[2][0] = a.x * a.z * (1.f - c) - a.y * s;
	m[2][1] = a.y * a.z * (1.f - c) + a.x * s;
	m[2][2] = a.z * a.z + (1.f - a.z * a.z) * c;
	m[2][3] = 0;

	m[3][0] = 0;
	m[3][1] = 0;
	m[3][2] = 0;
	m[3][3] = 1;

	Matrix4x4 mat(m);
	return Transform(mat, transpose(mat));
}

Transform Transform::look_at(const Point& pos, const Point& look, const Vec3& up)
{
	float m[4][4];
	m[0][3] = pos.x;
	m[1][3] = pos.y;
	m[2][3] = pos.z;
	m[3][3] = 1;

	auto dir = normalize(look - pos);
	auto left = normalize(cross(normalize(up), dir));
	auto new_up = cross(dir, left);
	m[0][0] = left.x;
	m[1][0] = left.y;
	m[2][0] = left.z;
	m[3][0] = 0;
	m[0][1] = new_up.x;
	m[1][1] = new_up.y;
	m[2][1] = new_up.z;
	m[3][1] = 0;
	m[0][2] = dir.x;
	m[1][2] = dir.y;
	m[2][2] = dir.z;
	m[3][2] = 0;
	auto camToWorld(m);
	return Transform(inverse(camToWorld), camToWorld);

}