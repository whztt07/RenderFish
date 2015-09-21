#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "MathHelper.h"
#include "Color.h"
#include "Ray.h"
#include "ShadeRec.h"

class Geometry
{
public:
	Color color;
	virtual ~Geometry() {}
	virtual bool hit(const Ray& ray, float& tmin, ShadeRec& out_hit_info) = 0;

	void set_color(const Color& color) { this->color = color; }
};


class Plane : public Geometry
{
public:
	Vec3 position;
	Vec3 normal;
	float d;

	Plane(const Vec3& position, const Vec3& normal) : position(position), normal(normal) {}
	Plane(const Vec3& normal, float d) : normal(normal), d(d) {
		position = normal * d;
	}

	virtual bool hit(const Ray& ray, float& tmin, ShadeRec& out_hit_info) override
	{
		float a = dot(ray.d, normal);
		if (a >= 0)
			return false;
		float b = dot(this->normal, ray.o - this->position);
		out_hit_info.geometry = this;
		//out_hit_info.distance = -b / a;
		//out_hit_info.position = ray.point_at(out_hit_info.distance);
		tmin = -b / a;
		out_hit_info.normal = this->normal;
	}
};


class Sphere : public Geometry
{
public:
	Point center;
	float radius;
	float square_radius;

	Sphere() : center(0, 0, 0), radius(1), square_radius(1) {}
	Sphere(const Point& center, float radius)
		: center(center), radius(radius), square_radius(radius * radius) {}

	void set_center(const Point& center) { this->center = center; };
	void set_radius(const float radius) { this->radius = radius; };

	virtual bool hit(const Ray& ray, float& tmin, ShadeRec& out_hit_info) override
	{
		auto v = ray.o - center;
		float a0 = v.length_squared() - radius * radius;
		float DdotV = dot(ray.d, v);

		if (DdotV <= 0)
		{
			float disc_r = DdotV * DdotV - a0;
			if (disc_r >= 0) {
				out_hit_info.geometry = this;
				tmin = -DdotV - sqrt(disc_r);
				//out_hit_info.distance = dist;
				auto position = ray(tmin);
				out_hit_info.normal = (position - center).normalize();
				return true;
			}
		}
		return false;
	}
};

#endif