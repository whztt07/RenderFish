#pragma once
#include "Scene.hpp"
#include "Ray.hpp"
#include "Sampler.hpp"
#include "MemoryArena.hpp"
#include "Spectrum.hpp"
#include "Camera.hpp"

class SurfaceIntegrator;

class Renderer
{
public:
	Renderer() {}

	virtual void render(const Scene * scene) const = 0;

	// returns the incident radiance along the given ray
	virtual Spectrum Li(const Scene *scene, const RayDifferential &ray,
		const Sample *sample, RNG &rng, MemoryArena &arena,
		Intersection *isect = nullptr, Spectrum *T = nullptr) const = 0;

	virtual Spectrum transmittance(const Scene *scene, const RayDifferential &ray,
		const Sample *sample, RNG &rng, MemoryArena &arena) const = 0;
};

//class SamplerRenderTask : public Task {
//
//};

class SamplerRender : public Renderer {
public:

	Sampler *sampler;
	PBRTCamera *camera;
	SurfaceIntegrator *suface_integrator;

	virtual void render(const Scene * scene) const override;

	SamplerRender(PBRTCamera* camera, SurfaceIntegrator* suface_integrator)
		: camera(camera), suface_integrator(suface_integrator) {
	}

	virtual Spectrum Li(const Scene *scene, const RayDifferential &ray,
		const Sample *sample, RNG &rng, MemoryArena &arena,
		Intersection *isect = nullptr, Spectrum *T = nullptr) const override {
		scene->intersect(ray, isect);
	}
	
	virtual Spectrum transmittance(const Scene *scene, const RayDifferential &ray,
		const Sample *sample, RNG &rng, MemoryArena &arena) const override {
		error("not implemented SimpleRender::transmittance");
	}
};
