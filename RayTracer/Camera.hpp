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
	
	virtual float gererate_ray_differential(const CameraSample &sample, RayDifferential *rd) const {
		float wt = generate_ray(sample, rd);
		CameraSample sshift = sample;
		++(sshift.image_x);
		Ray rx;
		float wtx = generate_ray(sshift, &rx);
		rd->rx_origin = rx.o;
		rd->rx_direction = rx.d;

		--(sshift.image_x);
		++(sshift.image_y);
		Ray ry;
		float wty = generate_ray(sshift, &ry);
		rd->ry_origin = ry.o;
		rd->ry_direction = ry.d;

		if (wtx == 0.f || wty == 0.f)
			return 0.f;
		rd->has_differentials = true;
		return wt;
	}
};

class ProjectiveCamera : public PBRTCamera {
protected:
	Transform camera_to_screen, raster_to_camera;
	Transform screen_to_raster, raster_to_screen;
public:
	ProjectiveCamera(const Transform &cam2world, const Transform &proj,
		const float screen_window[4],
		//float sopen, float sclose,
		float lensr, float focald, Film *film)
		: PBRTCamera(cam2world, film), camera_to_screen(proj) {
		
		// initialixe depth of field parameters


		screen_to_raster = scale(float(film->x_resolution), float(film->y_resolution), 1.f) *
			scale(1.f / (screen_window[1] - screen_window[0]), 1.f / (screen_window[2] - screen_window[3]), 1.f) *
			translate(Vec3(-screen_window[0], -screen_window[3], 0.f));
		raster_to_screen = inverse(screen_to_raster);
		raster_to_camera = inverse(camera_to_screen) * raster_to_screen;
	}
};

class OrthCamera : public ProjectiveCamera {
private:

public:
	OrthCamera(const Transform &cam2world, const Transform &proj,
		const float screen_window[4],
		//float sopen, float sclose,
		float lensr, float focald, Film *film)
		: ProjectiveCamera(cam2world, orthographic(0.f, 1.f), screen_window, lensr, focald, film) {
	}

};