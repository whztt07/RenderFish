#pragma once

#include "Math.hpp"
#include "Ray.hpp"
#include "Transform.hpp"

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

	Camera(const Point& eye, const Vec3& front, float aspect_ratio = 1.0f , float fov = 45.0f)
		: eye(eye), front(front), aspect_ratio(aspect_ratio), fov(fov)
	{
		fovScale = tanf(radians(fov * 0.5f)) * 2;
		right = cross(up, front);
		up = cross(front, right);
	}

	Ray ray_to(float x, float y)
	{
		auto r = right * ((x - 0.5f * aspect_ratio) * fovScale);
		auto u = up * ((y - 0.5f) * fovScale);
		return Ray(eye, normalize((front + r + u)));
	}
};


class PRTCamera
{
public:
	Transform camera_to_world;
	//const float shutter_open, shutter_close;
	//Film *film;

public:
	//PRTCamera(const Transform &cam2world, float sopen, float sclose, Film* film);
	PRTCamera(const Transform &cam2world)
		: camera_to_world(cam2world) {}

	//virtual float generate_ray(const CameraSample &sample, Ray *ray) const = 0;
};