#pragma once
#include "Math.hpp"
#include "Shape.hpp"

class DifferentialGeometry
{
public:
	Point p;
	Normal normal;
	float u = 0;
	float v = 0;
	const Shape * shape = nullptr;
	Vec3 dpdu, dpdv;
	Normal dndu, dndv;

	// barycentric coordinates
	// for Triangle only
	float b1, b2;

	DifferentialGeometry() {};
	DifferentialGeometry(const Point& p, const Vec3& dpdu, const Vec3& dpdv, 
		const Normal& dndu, const Normal& dndv,
		float u, float v, const Shape *shape)
		: p(p), dpdu(dpdu), dpdv(dpdv), dndu(dndu), dndv(dndv), u(u), v(v), shape(shape),
		normal(normalize(cross(dpdu, dpdv)))
	{
		if (shape != nullptr && (shape->reverse_orientation ^ shape->transform_swaps_handedness))
			normal *= -1;
	}
	~DifferentialGeometry() {};
};

