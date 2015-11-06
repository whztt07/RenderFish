#pragma once
#include "Scene.hpp"
#include "Ray.hpp"
#include "Sampler.hpp"
#include "MemoryArena.hpp"
#include "Spectrum.hpp"
#include "Parallel.hpp"

class SurfaceIntegrator;
class VolumeIntegrator;
class PBRTCamera;

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

private:
	Renderer(const Renderer&);
	Renderer& operator=(const Renderer&);
};

class SamplerRender : public Renderer 
{
private:
	Sampler				*m_sampler;
	PBRTCamera			*m_camera;
	SurfaceIntegrator	*m_suface_integrator;
	VolumeIntegrator	*m_volume_integrator;

public:
	virtual void render(const Scene * scene) override;

	SamplerRender(Sampler *sampler, PBRTCamera* camera, SurfaceIntegrator* suface_integrator, VolumeIntegrator* volume_integrator)
		: m_sampler(sampler), m_camera(camera), m_suface_integrator(suface_integrator), m_volume_integrator(volume_integrator) {
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
	const Scene		*m_scene;
	const Renderer	*m_renderer;
	PBRTCamera		*m_camera;
	Sampler			*m_main_sampler;
	Sample			*m_orignal_sample;
	int				m_task_number, m_task_count;

public:
	SamplerRenderTask(const Scene *scene, Renderer *renderer, PBRTCamera *camera, Sampler *main_sampler,
		Sample *orignal_sample, int task_number, int task_count)
		: m_scene(scene), m_renderer(renderer), m_camera(camera), m_main_sampler(main_sampler),
		m_orignal_sample(orignal_sample), m_task_number(task_number), m_task_count(task_count) {
	}

	virtual void run() override;
};
