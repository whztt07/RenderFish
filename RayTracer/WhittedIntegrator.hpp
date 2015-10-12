#pragma once
#include "Integrator.hpp"

class WhittedIntegrator : public SurfaceIntegrator
{
private:
	int _max_depth = 5;
public:
	WhittedIntegrator();
	~WhittedIntegrator();

	Spectrum Li(const Scene *scene, const Renderer *renderer, const RayDifferential &ray,
		const Intersection &isect, const Sample *sample, RNG &rng, MemoryArena &arena) const;
};

