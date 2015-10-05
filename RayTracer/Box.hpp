#pragma once
#include "Shape.hpp"
#include "DifferentialGeometry.hpp"

// in xy plane, and center on (0, 0, 0)
class Quad : public Shape
{
public:
	// plane: nv + d = 0
	//Normal n;
	//Vec3 up;
	//float d;
	float width, height;

public:
	Quad(const Transform *o2w, const Transform *w2o, bool reverse_orientation, float width, float height)
		: Shape(o2w, w2o, reverse_orientation), width(width), height(height) {
	}
	virtual BBox object_bound() const override{
		return BBox(Point(-width / 2.f, -height / 2.f, 0), Point(width / 2.f, height / 2.f, 0));
	}

	virtual bool intersect(const Ray &ray, float *t_hit, float *ray_epsilon,
		DifferentialGeometry *diff_geo) const override;

	virtual bool intersect_p(const Ray &ray) const override {
		Ray r;
		(*world_to_object)(ray, &r);
		if (zero(r.d.z))
			return false;
		float t = -r.o.z / r.d.z;
		auto p = r(t);
		if (t > r.maxt || t < r.mint)
			return false;
		if (p.x < -width / 2.f || p.x > width / 2.f || p.y < -height / 2.f || p.y > height / 2.f)
			return false;
		return true;
	}
};

class Box : public Shape
{
public:
	float width, height, depth;	// x, y, z;
	//static const Normal face_normals[6];

public:
	Box(float width = 1.f , float height = 1.f, float depth = 1.f)
		: width(width), height(height), depth(depth) {}
	
	virtual BBox object_bound() const override {
		return BBox(Point(-width / 2.f, -height / 2.f, -depth / 2.f), Point(width / 2.f, height / 2.f, depth / 2.f));
	}

	virtual bool can_intersect() const override {
		return false;
	}

	virtual void refine(vector<Reference<Shape>> &refined) const override;
};

