#include "MatteMaterial.hpp"
#include "Reflection.hpp"

BSDF * MatteMaterial::get_BSDF(const DifferentialGeometry & dg_geom, const DifferentialGeometry & dg_shading, MemoryArena & arena) const {

	DifferentialGeometry dgs;
	if (bump_map) {
		error("not bump_map\n");
	}
	else {
		dgs = dg_shading;
	}
	BSDF *bsdf = BSDF_ALLOC(arena, BSDF)(dgs, dg_geom.normal);

	// evaluate textures for MatteMaterial and allocate BRDF
	Spectrum r = Kd->evaluate(dgs).clamp();
	float sig = clamp(sigma->evaluate(dgs), 0.f, 90.f);
	if (sig == 0.f)
		bsdf->add(BSDF_ALLOC(arena, Lambertian)(r));
	//else
	//	bsdf->add(BSDF_ALLOC(arena, OrenNayar)(r, sig))

	return nullptr;
}
