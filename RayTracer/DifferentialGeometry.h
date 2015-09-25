#pragma once
#include "Math.hpp"
#include "Shape.h"

class DifferentialGeometry
{
public:
	Point p;
	Normal nn;
	float u = 0;
	float v = 0;
	const Shape * shape = nullptr;
	Vec3 dpdu, dpdv;
	Normal dndu, dndv;


	DifferentialGeometry() {};
	DifferentialGeometry(const Point& p, const Vec3& dpdu, const Vec3& dpdv, 
		const Normal& dndu, const Normal& dndv,
		float u, float v, const Shape *shape)
		: p(p), dpdu(dpdu), dpdv(dpdv), dndu(dndu), dndv(dndv), u(u), v(v), shape(shape),
		nn(normalize(cross(dpdu, dpdv)))
	{
		if (shape != nullptr && (shape->reverse_orientation ^ shape->transform_swaps_handedness))
			nn *= -1;
	}
	~DifferentialGeometry();
};

