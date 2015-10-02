#pragma once
#include "Shape.hpp"

class Sphere : public Shape {
private:
	float _radius;
	float _phi_max = radians(360);	// in radians
	float _z_min, _z_max;
	float _theta_min = M_PI;
	float _theta_max = 0;

public:
	Sphere(const Transform *obj2world, const Transform *world2obj, bool reverse_orientation,
		float radius)
		: Shape(obj2world, world2obj, reverse_orientation), _radius(radius), _z_min(-radius), _z_max(radius) {
	}

	Sphere(const Transform *obj2world, const Transform *world2obj, bool reverse_orientation,
		float radius, float z0, float z1, float phi_max)
		: Shape(obj2world, world2obj, reverse_orientation) {
		_radius = radius;
		_z_min = clamp(min(z0, z1), -radius, radius);
		_z_max = clamp(max(z0, z1), -radius, radius);
		_theta_min = acosf(clamp(_z_min / radius, -1.f, 1.f));
		_theta_max = acosf(clamp(_z_max / radius, -1.f, 1.f));
		_phi_max = radians(clamp(phi_max, 0.f, 360.f));
	}

	virtual BBox object_bound() const override {
		// P176 exercise 3.2
		float x_min = -_radius, y_max = _radius, y_min = -_radius;
		if (_phi_max < M_PI_4 * 3)
			y_min = _radius * sinf(_phi_max);
		if (_phi_max < M_PI_2) {
			x_min = _radius * cosf(_phi_max);
			y_min = 0;
		}
		if (_phi_max < M_PI_4) {
			y_max = _radius * sinf(_phi_max);
		}

		return BBox(Point(x_min, y_min, _z_min), Point(_radius, y_max, _z_max));
	}

	virtual bool intersect(const Ray &ray, float *t_hit, float *ray_epsilon,
		DifferentialGeometry *diff_geo) const override;

	virtual bool intersect_p(const Ray& r) const override;

	virtual float area() const {
		return _phi_max * _radius * (_z_max - _z_min);
	}
};

