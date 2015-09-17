#pragma once
#include "MathHelper.h"

class Ray
{
public:
	Vec3 origin, direction;

	Ray() : origin(0, 0, 0), direction(0, 0, -1) {}
	Ray(const Vec3& origin, const Vec3& direction) : origin(origin), direction(direction) {}
	Ray(const Ray& ray) : Ray(ray.origin, ray.direction) {}

	Vec3 point_at(float t) const { return origin + direction * t; }

	Ray& operator=(const Ray& rhs) { origin = rhs.origin; direction = rhs.direction; }
};