#pragma once

#include "RenderFish.hpp"
#include "Math.hpp"

void ConcentricSampleDisk(float u1, float u2, float *dx, float *dy);

inline Vec3 CosineSampleHemisphere(float u1, float u2) {
	Vec3 ret;
	ConcentricSampleDisk(u1, u2, &ret.x, &ret.y);
	ret.z = sqrtf(max(0.f, 1.f - ret.x*ret.x - ret.y*ret.y));
	return ret;
}

Vec3 UniformSampleHemisphere(float u1, float u2);