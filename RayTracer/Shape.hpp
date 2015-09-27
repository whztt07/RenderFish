#pragma once
#include "Transform.hpp"
#include "ReferenceCounted.hpp"
#include "Tracer.hpp"

class DifferentialGeometry;

class Shape : public ReferenceCounted
{
public:
	const Transform *object_to_world, *world_to_object;
	const bool reverse_orientation, transform_swaps_handedness;
	const uint32_t shape_id;
	static uint32_t next_shape_id;
	
	Shape(const Transform *o2w, const Transform *w2o, bool ro)
		: object_to_world(o2w), world_to_object(w2o), reverse_orientation(ro), 
		transform_swaps_handedness(o2w->swaps_handedness()), shape_id(next_shape_id++)  {
	}
	virtual ~Shape() {};

	virtual BBox object_bound() const = 0;
	BBox world_bound() const {
		return (*object_to_world)(object_bound());
	}

	bool can_intersect() const {
		return true;
	}

	void refine(vector<Reference<Shape>> &refine) const {
		error("Unimplemented Shaped::Refine() method called");
	}

	bool interset(const Ray &ray, float *t_hit, float *rayEpsilon,
		DifferentialGeometry *dg) const;

	bool interset_p(const Ray &ray) const {
		error("Unimplemented Shaped::interset_p() method called");
		return false;
	}

	virtual void get_shaing_geometry(const Transform &obj2world,
		const DifferentialGeometry &dg, DifferentialGeometry *dg_shading) const;

	float Area() const {
		error("Unimplemented Shaped::Area() method called");
	}
};

class Shere : public Shape {
public:
	Shere(const Transform *obj2world, const Transform *world2obj, bool ro,
		float radius, float z0, float z1, float phi_max)
		: Shape(obj2world, world2obj, ro) {
		_radius = radius;
		_zmin = clamp(min(z0, z1), -radius, radius);
		_zmax = clamp(max(z0, z1), -radius, radius);
		_theta_min = acosf(clamp(_zmin / radius, -1.f, 1.f));
		_theta_max = acosf(clamp(_zmax / radius, -1.f, 1.f));
		_phi_max = radians(clamp(phi_max, 0.f, 360.f));
	}

	BBox object_bound() const {
		return BBox(Point(-_radius, -_radius, _zmin), Point(_radius, _radius, _zmax));
	}

	bool interset(const Ray &ray, float *t_hit, float *rayEpsilon,
		DifferentialGeometry *dg) const {
		float phi;
		Point phit;

		float A = ray.d.x * ray.d.x + ray.d.y * ray.d.y + ray.d.z * ray.d.z;
		float B = 2 * (ray.d.x * ray.o.x + ray.d.y * ray.o.y + ray.d.z * ray.o.z);
		float C = ray.o.x * ray.o.x + ray.o.y * ray.o.y + ray.o.z * ray.o.z - _radius * _radius;
		float t0, t1;
		if (quadratic(A, B, C, &t0, &t1))
			return false;

		if (t0 > ray.maxt || t1 < ray.mint)
			return false;
		float thit = t0;
		if (t0 < ray.mint) {
			thit = t1;
			if (thit > ray.maxt) return false;
		}

		phit = ray(thit);
		if (phit.x == 0.f && phit.y == 0.f) phit.x = 1e-5f * _radius;
		phi = atan2f(phit.y, phit.x);
		if (phi < 0.) phi += 2.f * M_PI;

		if ((_zmin > -_radius && phit.z < _zmin) ||
			(_zmax <  _radius && phit.z > _zmax) || phi > _phi_max) {
			return false;
		}

		float u = phi / _phi_max;
		float theta = acosf(clamp(phit.z / _radius, -1.f, 1.f));
		float v = (theta - _theta_min) / (_theta_max - _theta_min);

		float z_radius = sqrtf(phit.x * phit.x + phit.y * phit.y);
		float inv_z_radius = 1.f / z_radius;
		float cos_phi = phit.x * inv_z_radius;
		float sin_phi = phit.y * inv_z_radius;
		Vec3 dpdu(-_phi_max * phit.y, _phi_max * phit.x, 0);
		Vec3 dpdv = (_theta_max - _theta_min) *
			Vec3(phit.z * cos_phi, phit.z * sin_phi, _radius * sinf(theta));

		return true;
	}

private:
	float _radius;
	float _phi_max;
	float _zmin, _zmax;
	float _theta_min, _theta_max;
};

