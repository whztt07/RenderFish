#pragma once
#include "Transform.hpp"
#include "ReferenceCounted.hpp"

class DifferentialGeometry;

// PBRT p121 Weinatiogarten equations
static void calc_dndu_dndv(const Vec3& dpdu, const Vec3& dpdv, const Vec3& d2Pduu,
	const Vec3& d2Pduv, const Vec3& d2Pdvv, Normal* dndu, Normal* dndv)
{
	float E = dot(dpdu, dpdu);
	float F = dot(dpdu, dpdv);
	float G = dot(dpdv, dpdv);
	auto N = normalize(cross(dpdu, dpdv));
	float e = dot(N, d2Pduu);
	float f = dot(N, d2Pduv);
	float g = dot(N, d2Pdvv);
	float invEGF2 = 1.0f / (E * G - F * F);
	*dndu = Normal((f*F - e*G) * invEGF2 * dpdu + (e*F - f*E) * invEGF2 * dpdv);
	*dndv = Normal((g*F - f*G) * invEGF2 * dpdu + (f*F - g*E) * invEGF2 * dpdv);
}

class Shape : public ReferenceCounted
{
protected:
	static uint32_t next_shape_id;
	const uint32_t shape_id;

public:
	const Transform *object_to_world, *world_to_object;
	const bool reverse_orientation, transform_swaps_handedness;

	vector<Reference<Shape>> refined_shapes;
	
	Shape() : object_to_world(&Transform::identity), world_to_object(&Transform::identity), 
		reverse_orientation(false), transform_swaps_handedness(false), shape_id(next_shape_id++) {
	}

	Shape(const Transform *o2w, const Transform *w2o, bool reverse_orientation)
		: object_to_world(o2w), world_to_object(w2o), reverse_orientation(reverse_orientation), 
		transform_swaps_handedness(o2w->swaps_handedness()), shape_id(next_shape_id++)  {
	}
	virtual ~Shape() {};

	virtual BBox object_bound() const = 0;
	virtual BBox world_bound() const {
		return (*object_to_world)(object_bound());
	}

	virtual bool can_intersect() const {
		return true;
	}

	virtual void refine(vector<Reference<Shape>> &refined) const {
		error("Unimplemented Shaped::Refine() method called\n");
	}

	virtual bool interset(const Ray &ray, float *t_hit, float *ray_epsilon,
		DifferentialGeometry *diff_geo) const {
		error("Unimplemented Shaped::interset() method called\n");
		return false;
	}

	virtual bool interset_p(const Ray &ray) const {
		error("Unimplemented Shaped::interset_p() method called\n");
		return false;
	}

	virtual void get_shading_geometry(const Transform &obj2world,
		const DifferentialGeometry &dg, DifferentialGeometry *dg_shading) const;

	virtual float area() const {
		error("Unimplemented Shaped::Area() method called\n");
		return 0;
	}
};

class Sphere : public Shape {
private:
	float _radius;
	float _phi_max = radians(360);
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

	virtual bool interset(const Ray &ray, float *t_hit, float *ray_epsilon,
		DifferentialGeometry *diff_geo) const;

	virtual bool interset_p(const Ray& r) const override;

	virtual float area() const {
		return _phi_max * _radius * (_z_max - _z_min);
	}
};

