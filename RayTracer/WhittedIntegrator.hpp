#pragma once
#include "Integrator.hpp"
#include "Material.hpp"

class WhittedIntegrator : public SurfaceIntegrator
{
private:
	int _max_depth = 5;
public:
	WhittedIntegrator();
	~WhittedIntegrator();

	Spectrum Li(const Scene *scene, const Renderer *renderer, const RayDifferential &ray,
		const Intersection &isect, const Sample *sample, RNG &rng, MemoryArena &arena) const {

		Spectrum L(0.);
		// compute emitted and reflected light at ray intersection point
		// evaluate BSDF at hit point
		BSDF *bsdf = isect.get_BSDF(ray, arena);
		// initialize common variables for Whitted integrator
		const Point &p = bsdf->dg_shading.p;
		const Normal &n = bsdf->dg_shading.normal;
		Vec3 wo = -ray.d;
		// compute emitted light if ray hit an area light source
		// add contribution of each light source
		if (ray.depth + 1 < _max_depth) {
			// trace rays for specular reflection and refraction
		}
		return L;
	}
};

