#include "Intersection.hpp"
#include "Primitive.hpp"

BSDF * Intersection::get_BSDF(const RayDifferential & ray, MemoryArena & arena) const {
	dg.compute_differentials(ray);
	BSDF *bsdf = primitive->get_BSDF(dg, object_to_world, arena);
	return bsdf;
}
