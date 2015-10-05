#pragma once
#include "Scene.hpp"
#include "Ray.hpp"
#include "Sampler.hpp"
#include "MemoryArena.hpp"
#include "Spectrum.hpp"

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

class SamplerRender : public Renderer {
private:
	Sampler *sampler;
	//Camera *camera;
public:

};