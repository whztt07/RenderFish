#pragma once
#include "Material.hpp"
#include "Texture.hpp"
#include "Spectrum.hpp"

class BSDF;

class MatteMaterial : public Material
{
private:
	Reference<Texture<Spectrum>> Kd;
	Reference<Texture<float>> sigma, bump_map;

public:

	MatteMaterial(Reference<Texture<Spectrum>> kd,
		Reference<Texture<float>> sigma, Reference<Texture<float>> bump_map)
		: Kd(kd), sigma(sigma), bump_map(bump_map) {
	}

	BSDF* get_BSDF(const DifferentialGeometry &dg_geom, 
		const DifferentialGeometry &dg_shading, MemoryArena &arena) const;
};

