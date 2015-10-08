#pragma once

#include "Math.hpp"
#include "Ray.hpp"
#include "Transform.hpp"
#include "Film.hpp"
#include "Sampler.hpp"

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

	Camera(const Point& eye, const Vec3& front, float aspect_ratio = 1.0f , float fov = 45.0f);

	Ray ray_to(float x, float y);
};


class PBRTCamera
{
public:
	Transform camera_to_world;
	//const float shutter_open, shutter_close;
	Film *film;

public:
	//PRTCamera(const Transform &cam2world, float sopen, float sclose, Film* film);
	PBRTCamera(const Transform &cam2world, Film *film)
		: camera_to_world(cam2world), film(film) {}

	//  returns a floating-point value that gives a weight for how much light
	//     arriving at the film plane along the generated ray will contribute to the final image
	virtual float generate_ray(const CameraSample &sample, Ray *ray) const = 0;
	
	virtual float gererate_ray_differential(const CameraSample &sample, RayDifferential *rd) const;
};

class ProjectiveCamera : public PBRTCamera {
protected:
	Transform camera_to_screen, raster_to_camera;
	Transform screen_to_raster, raster_to_screen;
public:
	ProjectiveCamera(const Transform &cam2world, const Transform &proj,
		const float screen_window[4],
		//float sopen, float sclose,
		float lensr, float focald, Film *film);
};

class OrthoCamera : public ProjectiveCamera {
private:
	Vec3 dx_camera, dy_camera;
public:
	OrthoCamera(const Transform &cam2world, const Transform &proj,
		const float screen_window[4],
		//float sopen, float sclose,
		float lensr, float focald, Film *film)
		: ProjectiveCamera(cam2world, orthographic(0.f, 1.f), screen_window, lensr, focald, film) {

		dx_camera = raster_to_camera(Vec3(1, 0, 0));
		dy_camera = raster_to_camera(Vec3(0, 1, 0));
	}

	virtual float generate_ray(const CameraSample &sample, Ray *ray) const override;

	virtual float gererate_ray_differential(const CameraSample &sample, RayDifferential *rd) const override;

};


class PerspectiveCamera: public ProjectiveCamera{
private:
	Vec3 dx_camera, dy_camera;

public:
	PerspectiveCamera(const Transform &cam2world, const Transform &proj,
		const float screen_window[4],
		//float sopen, float sclose,
		float lensr, float focald, float fov, Film *film);

	virtual float generate_ray(const CameraSample &sample, Ray *ray) const override;

	virtual float gererate_ray_differential(const CameraSample &sample, RayDifferential *rd) const override;
};