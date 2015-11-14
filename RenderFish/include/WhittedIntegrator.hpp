#pragma once
#include "Integrator.hpp"

class WhittedIntegrator : public SurfaceIntegrator
{
private:
	int m_max_depth = 5;
public:
	WhittedIntegrator(int max_depth = 5) : m_max_depth(max_depth) {};
	//~WhittedIntegrator() {};

	Spectrum Li(const Scene *scene, const Renderer *renderer, const RayDifferential &ray,
		const Intersection &isect, const Sample *sample, RNG &rng, MemoryArena &arena) const;
};

