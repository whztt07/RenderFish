#pragma once
#include "Scene.hpp"
#include "Ray.hpp"
#include "Sampler.hpp"
#include "MemoryArena.hpp"
#include "Spectrum.hpp"
#include "Camera.hpp"
#include "Parallel.hpp"

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

class SamplerRenderTask: public Task
{
private:
	const Scene		*scene;
	const Renderer	*renderer;
	Camera			*camera;
	Sampler			*main_sampler;
	Sample			*orignal_sample;
	int				task_number, task_count;

public:
	SamplerRenderTask(const Scene *scene, Renderer *renderer, Camera *camera, Sampler *main_sampler,
		Sample *orignal_sample, int task_number, int task_count)
		: scene(scene), renderer(renderer), camera(camera), main_sampler(main_sampler),
		orignal_sample(orignal_sample), task_number(task_number), task_count(task_count) {
	}

	virtual void run() override {

	}

}