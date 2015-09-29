#pragma once
#include "Transform.hpp"
#include "ReferenceCounted.hpp"

class DifferentialGeometry;

class Shape : public ReferenceCounted
{
public:
	const Transform *object_to_world, *world_to_object;
	const bool reverse_orientation, transform_swaps_handedness;
	const uint32_t shape_id;
	static uint32_t next_shape_id;
	
	Shape(const Transform *o2w, const Transform *w2o, bool reverse_orientation)
		: object_to_world(o2w), world_to_object(w2o), reverse_orientation(reverse_orientation), 
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

	virtual bool interset(const Ray &ray, float *t_hit, float *rayEpsilon,
		DifferentialGeometry *dg) const = 0;

	virtual bool interset_p(const Ray &ray) const = 0 {
		error("Unimplemented Shaped::interset_p() method called");
		return false;
	}

	virtual void get_shading_geometry(const Transform &obj2world,
		const DifferentialGeometry &dg, DifferentialGeometry *dg_shading) const;

	float Area() const {
		error("Unimplemented Shaped::Area() method called");
	}
};

class Sphere : public Shape {
private:
	float _radius;
	float _phi_max = 360;
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

	BBox object_bound() const {
		return BBox(Point(-_radius, -_radius, _z_min), Point(_radius, _radius, _z_max));
	}

	virtual bool interset(const Ray &r, float *t_hit, float *rayEpsilon,
		DifferentialGeometry *diff_geo) const override;

	virtual bool interset_p(const Ray& r) const override;

};

