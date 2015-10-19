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

public:
	const uint32_t shape_id;
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

	virtual bool intersect(const Ray &ray, float *t_hit, float *ray_epsilon,
		DifferentialGeometry *diff_geo) const {
		error("Unimplemented Shaped::interset() method called\n");
		return false;
	}

	virtual bool intersect_p(const Ray &ray) const {
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
