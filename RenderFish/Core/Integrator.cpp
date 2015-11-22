#include "Integrator.hpp"
#include "Light.hpp"
#include "Reflection.hpp"

Spectrum NDotLIntegrator::Li(const Scene *scene, const Renderer *renderer, const RayDifferential &ray, const Intersection &isect, const Sample *sample, RNG &rng, MemoryArena &arena) const
{
	Spectrum L(0.f);
	auto bsdf = isect.get_BSDF(ray, arena);
	const Normal& N = isect.dg.normal;
	const Point& pos = isect.dg.p;
	Vec3 wo = -ray.d;
	// add contribution of each light source
	for (Light* light : scene->lights) {
		Vec3 wi;
		float pdf;
		VisibilityTester visibility;
		Spectrum Li = light->sample_L(pos, isect.ray_epsilon, LightSample(rng), &wi, &pdf, &visibility);
		if (Li.is_black() || pdf == 0.f) continue;
		Spectrum f = bsdf->f(wo, wi);
		if (!f.is_black() && visibility.unoccluded(scene)) {
			L += f * Li * fabsf(dot(wi, N)) / pdf;
		}
	}
	return L;
}
