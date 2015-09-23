#ifndef RAY_H
#define RAY_H
#include "Math.hpp"

class Ray
{
public:
	Point			o;			// origin
	Vec3			d;			// direction
	mutable float	mint = 0.0f;
	mutable float	maxt = INFINITY;
	float			time = 0.0f;
	int				depth = 0;	// bounce times

	Ray() {}
	Ray(const Point& origin, const Vec3& direction)
		: o(origin), d(direction) {}
	Ray(const Point & origin, const Vec3 & direction, float start, float end = INFINITY, float t = 0.0f, int d = 0)
		: o(origin), d(direction), mint(start), maxt(end), time(t), depth(d) {}
	Ray(const Ray& ray) 
		: Ray(ray.o, ray.d) {}

	// spawn from a "parent" ray
	Ray(const Point& origin, const Vec3& direction, const Ray& parent, float start, float end = INFINITY)
		: o(origin), d(direction), mint(start), maxt(end), time(parent.time), depth(parent.depth + 1) {}

	//Point point_at(float t) const { return o + d * t; }
	Point operator()(float t) const { return o + d * t; }

	Ray& operator=(const Ray& rhs) { o = rhs.o; d = rhs.d; }
};

class RayDifferential : public Ray
{
public:
	bool has_differentials = false;
	Point rx_origin, ry_origin;
	Vec3 rx_direction, ry_direction;

	RayDifferential() {}
	RayDifferential(const Ray& ray) 
		: Ray(ray) {}
	RayDifferential(const Point & org, const Vec3 & dir, float start, float end = INFINITY, float t = 0.0f, int d = 0)
		: Ray(org, dir, start, end, t, d) {}

	void scale_differentials(float s)
	{
		rx_origin = o + (rx_origin - o) * s;
		ry_origin = o + (ry_origin - o) * s;
		rx_direction = d + (rx_direction - d) * s;
		ry_direction = d + (ry_direction - d) * s;
	}
};

#endif // RAY_H