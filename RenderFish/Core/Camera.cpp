#include "Camera.hpp"
#include "Film.hpp"


float Camera::gererate_ray_differential(const CameraSample &sample, RayDifferential *rd) const
{
	float weight = generate_ray(sample, rd);
	// shift one pixel in x direction
	CameraSample sshift = sample;
	++(sshift.image_x);
	Ray rx;
	float weight_x = generate_ray(sshift, &rx);
	rd->rx_origin = rx.o;
	rd->rx_direction = rx.d;

	// shift one pixel in y direction
	--(sshift.image_x);
	++(sshift.image_y);
	Ray ry;
	float weight_y = generate_ray(sshift, &ry);
	rd->ry_origin = ry.o;
	rd->ry_direction = ry.d;

	if (weight_x == 0.f || weight_y == 0.f)
		return 0.f;
	rd->has_differentials = true;
	return weight;
}

ProjectiveCamera::ProjectiveCamera(const Transform & cam2world, const Transform & proj, const float screen_window[4], Film * film)
	: Camera(cam2world, film), camera_to_screen(proj) {

	screen_to_raster =  scale(float(film->x_resolution), float(film->y_resolution), 1.f) *
						scale(  1.f / (screen_window[1] - screen_window[0]), 
								1.f / (screen_window[2] - screen_window[3]), 1.f) *
						translate(Vec3(-screen_window[0], -screen_window[3], 0.f));
	raster_to_screen = inverse(screen_to_raster);
	raster_to_camera = inverse(camera_to_screen) * raster_to_screen;
}

float OrthoCamera::generate_ray(const CameraSample & sample, Ray * ray) const {
	Point p_ras(sample.image_x, sample.image_y, 0);
	Point p_camera;
	raster_to_camera(p_ras, &p_camera);
	*ray = Ray(p_camera, Vec3(0, 0, 1), 0.f, INFINITY);

	// modify ray for depth of field
	//ray->time = lerp(sample.time, )
	camera_to_world(*ray, ray);
	return 1.f;
}

float OrthoCamera::gererate_ray_differential(const CameraSample & sample, RayDifferential * rd) const {
	rd->rx_origin = rd->o + dx_camera;
	rd->ry_origin = rd->o + dy_camera;
	rd->rx_direction = rd->ry_direction = rd->d;
	rd->has_differentials = true;
	camera_to_world(*rd, rd);
	return 1.f;
}

OrthoCamera::OrthoCamera(const Transform &cam2world, const Transform &proj, const float screen_window[4], Film *film) : ProjectiveCamera(cam2world, orthographic(0.f, 1.f), screen_window, film)
{
	dx_camera = raster_to_camera(Vec3(1, 0, 0));
	dy_camera = raster_to_camera(Vec3(0, 1, 0));
}
PerspectiveCamera::PerspectiveCamera(const Transform & cam2world, const float screen_window[4], float fov, Film * film)
	: ProjectiveCamera(cam2world, perspective(fov, 1e-2f, 1000.f), screen_window, film) {
	dx_camera = raster_to_camera(Point(1, 0, 0)) - raster_to_camera(Point(0, 0, 0));
	dy_camera = raster_to_camera(Point(0, 1, 0)) - raster_to_camera(Point(0, 0, 0));
}

float PerspectiveCamera::generate_ray(const CameraSample &sample, Ray *ray) const
{
	Point p_ras(sample.image_x, sample.image_y, 0);
	Point p_camera;
	raster_to_camera(p_ras, &p_camera);
	*ray = Ray(Point(0, 0, 0), normalize(Vec3(p_camera)), 0.f, INFINITY);
	// modify ray for depth of field
	// TODO

	//ray->time = lerp(sample)
	camera_to_world(*ray, ray);
	return 1.f;
}

float PerspectiveCamera::gererate_ray_differential(const CameraSample &sample, RayDifferential *rd) const
{
	if (sample.image_x == 400 && sample.image_y == 300)
	{
		info("here\n");
	}
	Point p_ras(sample.image_x, sample.image_y, 0);
	Point p_camera;
	raster_to_camera(p_ras, &p_camera);
	*rd = RayDifferential(Point(0, 0, 0), normalize(Vec3(p_camera)), 0.f, INFINITY);
	// modify ray for depth of field

	rd->rx_origin = rd->ry_origin = rd->o;
	rd->rx_direction = normalize(Vec3(p_camera) + dx_camera);
	rd->ry_direction = normalize(Vec3(p_camera) + dy_camera);
	rd->has_differentials = true;
	//ray->time = lerp(sample)
	camera_to_world(*rd, rd);
	rd->has_differentials = true;
	return 1.f;
}
