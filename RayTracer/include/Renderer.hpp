#pragma once
#include "Scene.hpp"
#include "Ray.hpp"
#include "Sampler.hpp"
#include "MemoryArena.hpp"
#include "Spectrum.hpp"
#include "Camera.hpp"

class Renderer
{
public:
	Renderer() {}

	virtual void Render(const Scene * scene) = 0;

	// returns the incident radiance along the given ray
	virtual void Li(const Scene *scene, const RayDifferential &ray,
		const Sample *sample, RNG &rng, MemoryArena &arena,
		Intersection *isect = nullptr, Spectrum *T = nullptr) const = 0;

	virtual Spectrum transmittance(const Scene *scene, const RayDifferential &ray,
		const Sample *sample, RNG &rng, MemoryArena &arena) const = 0;
};

class SimpleRender : public Renderer {
public:

	PBRTCamera *camera;

	virtual void Li(const Scene *scene, const RayDifferential &ray,
		const Sample *sample, RNG &rng, MemoryArena &arena,
		Intersection *isect = nullptr, Spectrum *T = nullptr) const override {
		scene->intersect(ray, isect);
	}

	void render() {
		//for (int i = 0; i < 800; i++)
		//	for (int j = 0; j < 600; j++) {
		//		CameraSample sample;
		//		camera->generate_ray()
		//	}

	}
};

class SamplerRender : public Renderer {
private:
	Sampler *sampler;
	PBRTCamera *camera;
	//SurfaceIntegrator *surfaceIntegrator;
	//VolumeIntegrator *volumeIntegrator;
public:

};