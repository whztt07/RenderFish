#pragma once
#include "Scene.hpp"
#include "Camera.hpp"
#include "Renderer.hpp"

class Integrator
{
public:
	Integrator() {};

	virtual void preprocess(const Scene *scene, const PBRTCamera *camera, const Renderer *renderer) {};
	virtual void request_samples(Sampler *sampler, Sample *sample, const Scene* scene) {};

};


class SurfaceIntegrator : public Integrator {
public:
	virtual Spectrum Li(const Scene *scene, const Renderer *renderer, const RayDifferential &ray,
		const Intersection &isect, const Sample *sample, RNG &rng, MemoryArena &arena) const = 0;
};

class VolumeIntegrator : public Integrator {

};