#pragma once

#include "Math.hpp"
#include "Ray.hpp"

class Camera
{
public:
	Point eye;
	Vec3 front;
	Vec3 right;
	Vec3 up = Vec3(0, 1, 0);

	float z_far = 0.1f;
	float z_near = 100.0f;
	float fov = 45.0f;
	float aspect_ratio = 1.0f;
	float fovScale;

	Camera(const Point& eye, const Vec3& front, float aspect_ratio, float fov)
		: eye(eye), front(front)
	{
		fovScale = tanf(radians(fov * 0.5f)) * 2;
		right = cross(up, front);
		up = cross(front, right);
	}

	Ray ray_to(float x, float y)
	{
		auto r = right * ((x - 0.5f) * fovScale);
		auto u = up * ((y - 0.5f) * fovScale);
		return Ray(eye, normalize((front + r + u)));
	}
};