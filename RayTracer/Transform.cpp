#include "Transform.hpp"

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