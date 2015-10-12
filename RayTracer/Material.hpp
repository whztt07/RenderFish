#pragma once
#include "ReferenceCounted.hpp"

class BSDF;
class DifferentialGeometry;
class MemoryArena;

class Material : public ReferenceCounted
{
public:
	Material();
	virtual ~Material();

	virtual BSDF *get_BSDF(const DifferentialGeometry &dg_geom, const DifferentialGeometry &dg_shading,
		MemoryArena &arena) const = 0;
};