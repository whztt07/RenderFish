#pragma once
#include "Transform.hpp"
#include "Quaternion.hpp"

class AnimatedTransform
{
private:
	const float _start_time, _end_time;
	const Transform *_start_transform, *_end_transform;
	const bool _actually_animated;
	Vec3		T[2];
	Quaternion	R[2];
	Matrix4x4	S[2];

public:
	AnimatedTransform(const Transform *transform1, float time1,
		const Transform *transform2, float time2)
		: _start_time(time1), _end_time(time2),
		_start_transform(transform1), _end_transform(transform2),
		_actually_animated(*_start_transform != *_end_transform) {

		decompose(_start_transform->m, &T[0], &R[0], &S[0]);
		decompose(_end_transform->m, &T[1], &R[1], &S[1]);
	}

	void decompose(const Matrix4x4 &m, Vec3 * T, Quaternion * Rquat, Matrix4x4 *S);

	void interpolate(float time, Transform *t) const;

	void operator()(const Ray &r, Ray *tr) const;
	void operator()(const RayDifferential &r, RayDifferential *tr) const;
	Point operator()(float time, const Point &p) const;
	Vec3 operator()(float time, const Vec3 &v) const;
	Ray operator()(const Ray &r) const;

	BBox motion_bounds(const BBox &b, bool use_inverse) const;
};
