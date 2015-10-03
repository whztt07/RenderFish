#pragma once
#include "Vector.hpp"
#include "Transform.hpp"

class Quaternion
{
public:
	Vec3 v;
	float w;
public:
	Quaternion(): v(0, 0, 0), w(1.f) {}
	Quaternion(Vec3 v, float w) : v(v), w(w) {}
	Quaternion(const Transform &t);

	friend Quaternion operator+(const Quaternion &q1, const Quaternion &q2) {
		return Quaternion(q1.v + q2.v, q1.w + q2.w);
	}
	friend Quaternion operator-(const Quaternion &q1, const Quaternion &q2) {
		return Quaternion(q1.v - q2.v, q1.w - q2.w);
	}
	friend Quaternion operator*(float f, const Quaternion &q) {
		return Quaternion(f * q.v, f * q.w);
	}
	friend Quaternion operator*(const Quaternion &q, float f) {
		return Quaternion(f * q.v, f * q.w);
	}
	void operator+=(const Quaternion &q) { v += q.v; w += q.w; }
	void operator-=(const Quaternion &q) { v -= q.v; w -= q.w; }
	friend Quaternion operator/(const Quaternion &q, float f) {
		return Quaternion(q.v / f, q.w / f);
	}

	friend float dot(const Quaternion &q1, const Quaternion &q2) {
		return dot(q1.v, q2.v) + q1.w * q2.w;
	}

	friend Quaternion normalize(const Quaternion &q) {
		return q / sqrtf(dot(q, q));
	}

	Transform to_transform() const;

	friend Quaternion slerp(float t, const Quaternion &q1, const Quaternion &q2) {
		float cos_theta = dot(q1, q2);
		if (cos_theta > .9995f) {
			return normalize((1.f - t) * q1 + t * q2);
		}
		else {	// nearly parallel
			float theta = acosf(clamp(cos_theta, -1.f, 1.f));
			float thetap = theta * t;
			Quaternion qprep = normalize(q2 - q1 * cos_theta);
			return q1 * cosf(thetap) + qprep * sinf(thetap);
		}
	}
};

