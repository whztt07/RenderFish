#pragma once
#include "Scene.hpp"
#include "Ray.hpp"
#include "MemoryArena.hpp"
#include "Spectrum.hpp"

class Renderer
{
public:
	Renderer();
	virtual ~Renderer();

	virtual void Render(const Scene * scene) = 0;
	// returns the incident radiance along the given ray
	//virtual void Li(const Scene *scene, const RayDifferential &ray,
	//	const Sample *sample, RNG &rng, MemoryArena &arena,
	//	Intersection *isect = nullptr, Spectrum *T = nullptr) const = 0;

};

