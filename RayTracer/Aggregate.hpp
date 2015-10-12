#pragma once
#include "Primitive.hpp"
class Aggregate : public Primitive
{
public:
	Aggregate();
	virtual ~Aggregate();

	//virtual const AreaLight *get_area_light() const override {}
	//virtual BSDF *get_BSDF(const DifferentialGeometry &dg,
	//	const Transform &ObjectToWorld, MemoryArena &arena)const override {};
};

