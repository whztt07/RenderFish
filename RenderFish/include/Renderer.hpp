#pragma once
#include "Scene.hpp"
#include "Ray.hpp"
#include "Sampler.hpp"
#include "MemoryArena.hpp"
#include "Spectrum.hpp"
#include "Parallel.hpp"
#include "Camera.hpp"

class SurfaceIntegrator;

class Renderer
{
public:
	Renderer() {}

	virtual void render(const Scene * scene) = 0;

	// returns the incident radiance along the given ray
	virtual Spectrum Li(const Scene *scene, const RayDifferential &ray,
		const Sample *sample, RNG &rng, MemoryArena &arena,
		Intersection *isect = nullptr, Spectrum *T = nullptr) const = 0;

	virtual Spectrum transmittance(const Scene *scene, const RayDifferential &ray,
		const Sample *sample, RNG &rng, MemoryArena &arena) const = 0;
};

class SamplerRender : public Renderer 
{
private:
	Sampler				*sampler;
	PBRTCamera			*camera;
	SurfaceIntegrator	*suface_integrator;
	VolumeIntegrator	*volume_integrator;

public:
	virtual void render(const Scene * scene) override;

	SamplerRender(PBRTCamera* camera, SurfaceIntegrator* suface_integrator)
		: camera(camera), suface_integrator(suface_integrator) {
	}

	virtual Spectrum Li(const Scene *scene, const RayDifferential &ray,
		const Sample *sample, RNG &rng, MemoryArena &arena,
		Intersection *isect = nullptr, Spectrum *T = nullptr) const override;
	
	virtual Spectrum transmittance(const Scene *scene, const RayDifferential &ray,
		const Sample *sample, RNG &rng, MemoryArena &arena) const override {
		error("not implemented SimpleRender::transmittance");
		//return volume_integrator->transmittance(scene, this, ray, sample, rng, arena);
	}
};

class SamplerRenderTask: public Task
{
private:
	const Scene		*_scene;
	const Renderer	*_renderer;
	PBRTCamera		*_camera;
	Sampler			*_main_sampler;
	Sample			*_orignal_sample;
	int				_task_number, _task_count;

public:
	SamplerRenderTask(const Scene *scene, Renderer *renderer, PBRTCamera *camera, Sampler *main_sampler,
		Sample *orignal_sample, int task_number, int task_count)
		: _scene(scene), _renderer(renderer), _camera(camera), _main_sampler(main_sampler),
		_orignal_sample(orignal_sample), _task_number(task_number), _task_count(task_count) {
	}

	virtual void run() override;
};
