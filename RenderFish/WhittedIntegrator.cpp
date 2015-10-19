#include "WhittedIntegrator.hpp"
#include "Reflection.hpp"
#include "Light.hpp"
#include "Material.hpp"


WhittedIntegrator::WhittedIntegrator()
{
}


WhittedIntegrator::~WhittedIntegrator()
{
}

Spectrum WhittedIntegrator::Li(const Scene * scene, const Renderer * renderer, const RayDifferential & ray, const Intersection & isect, const Sample * sample, RNG & rng, MemoryArena & arena) const {

	Spectrum L(0.);
	// compute emitted and reflected light at ray intersection point
	// evaluate BSDF at hit point
	BSDF *bsdf = isect.get_BSDF(ray, arena);
	// initialize common variables for Whitted integrator
	const Point &p = bsdf->shading_diff_geom.p;
	const Normal &n = bsdf->shading_diff_geom.normal;
	Vec3 wo = -ray.d;
	// compute emitted light if ray hit an area light source
	L += isect.Le(wo);
	// add contribution of each light source
	for (uint32_t i = 0; i < scene->lights.size(); ++i) {
		Vec3 wi;
		float pdf;
		VisibilityTester visibility;
		Spectrum Li = scene->lights[i]->sample_L(p, isect.ray_epsilon, LightSample(rng),
			ray.time, &wi, &pdf, &visibility);
		if (Li.is_black() || pdf == 0.f) continue;
		Spectrum f = bsdf->f(wo, wi);
		if (!f.is_black() && visibility.unoccluded(scene)) {
			L += f * Li * fabsf(dot(wi, n)) *
				visibility.transmittance(scene, renderer, sample, rng, arena) / pdf;
		}
	}

	if (ray.depth + 1 < _max_depth) {
		// trace rays for specular reflection and refraction
		L += specular_reflect(ray, bsdf, rng, isect, renderer, scene, sample, arena);
		//L += SpecularTransmit(ray, bsdf, rng, isect, renderer, scene, sample, arena);
	}
	return L;
}
