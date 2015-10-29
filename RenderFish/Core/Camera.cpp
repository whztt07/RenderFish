#include "Camera.hpp"
#include "Film.hpp"

Camera::Camera(const Point& eye, const Vec3& front, float aspect_ratio /*= 1.0f */, float fov /*= 45.0f*/) : eye(eye), front(front), aspect_ratio(aspect_ratio), fov(fov)
{
	fovScale = tanf(radians(fov * 0.5f)) * 2;
	right = cross(up, front);
	up = cross(front, right);
}

Ray Camera::ray_to(float x, float y)
{
	auto r = right * ((x - 0.5f * aspect_ratio) * fovScale);
	auto u = up * ((y - 0.5f) * fovScale);
	return Ray(eye, normalize((front + r + u)));
}


float PBRTCamera::gererate_ray_differential(const CameraSample &sample, RayDifferential *rd) const
{
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

ProjectiveCamera::ProjectiveCamera(const Transform & cam2world, const Transform & proj, const float screen_window[4], float lensr, float focald, Film * film)
	: PBRTCamera(cam2world, film), camera_to_screen(proj) {

	// initialixe depth of field parameters


	screen_to_raster = scale(float(film->x_resolution), float(film->y_resolution), 1.f) *
		scale(1.f / (screen_window[1] - screen_window[0]), 1.f / (screen_window[2] - screen_window[3]), 1.f) *
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

PerspectiveCamera::PerspectiveCamera(const Transform & cam2world, const Transform & proj, const float screen_window[4], float lensr, float focald, float fov, Film * film)
	: ProjectiveCamera(cam2world, perspective(fov, 1e-2f, 1000.f), screen_window, lensr, focald, film) {
	dx_camera = raster_to_camera(Point(1, 0, 0)) - raster_to_camera(Point(0, 0, 0));
	dy_camera = raster_to_camera(Point(0, 1, 0)) - raster_to_camera(Point(0, 0, 0));
}

float PerspectiveCamera::generate_ray(const CameraSample &sample, Ray *ray) const
{
	Point p_ras(sample.image_x, sample.image_y, 0);
	Point p_camera;
	raster_to_camera(p_ras, &p_camera);
	*ray = Ray(Point(0, 0, 0), Vec3(p_camera), 0.f, INFINITY);
	// modify ray for depth of field

	//ray->time = lerp(sample)
	camera_to_world(*ray, ray);
	return 1.f;
}

float PerspectiveCamera::gererate_ray_differential(const CameraSample &sample, RayDifferential *rd) const
{
	Point p_ras(sample.image_x, sample.image_y, 0);
	Point p_camera;
	raster_to_camera(p_ras, &p_camera);
	//*ray = Ray(Point(0, 0, 0), Vec3(p_camera), 0.f, INFINITY);
	// modify ray for depth of field

	rd->rx_origin = rd->ry_origin = rd->o;
	rd->rx_direction = normalize(Vec3(p_camera) + dx_camera);
	rd->ry_direction = normalize(Vec3(p_camera) + dy_camera);
	rd->has_differentials = true;
	//ray->time = lerp(sample)
	camera_to_world(*rd, rd);
	return 1.f;
}
