#pragma once
#include "Transform.hpp"
#include "Spectrum.hpp"
#include "Scene.hpp"
#include "Renderer.hpp"
#include "Sampler.hpp"

struct LightSample {
public:
	float u_pos[2];
	float u_component;

	LightSample() {}
	//LightSample(const Sample *sample, const LightSampleOffsets &offsets, uint32_t num);
	LightSample(RNG &rng) {
		u_pos[0] = rng.random_float();
		u_pos[1] = rng.random_float();
		u_component = rng.random_float();
	}
};

struct VisibilityTester {
	Ray r;
	void set_segment(const Point &p1, float eps1, const Point &p2, float eps2, float time) {
		float dist = distance(p1, p2);
		r = Ray(p1, (p2 - p1) / dist, eps1, dist*(1.f - eps2), time);
	}
	void set_ray(const Point &p, float eps, const Vec3 &w, float time) {
		r = Ray(p, w, eps, INFINITY, time);
	}
	bool unoccluded(const Scene *scene) const {
		return !scene->intersect_p(r);
	}
	Spectrum transmittance(const Scene* scene, const Renderer *renderer,
		const Sample *sample, RNG &rng, MemoryArena &arena) const {
		return renderer->transmittance(scene, RayDifferential(r), sample, rng, arena);
	}
};

class Light
{
protected:
	const Transform light_to_world, world_to_light;

public:
	const int n_samples;

public:
	Light(const Transform &light_to_world, int n_samples = 1)
		: n_samples(max(1, n_samples)), light_to_world(light_to_world),
		  world_to_light(inverse(light_to_world)) {
		Assert(!light_to_world.has_scale());
	}

	virtual Spectrum sample_L(const Point &p_w, float pEpsilon,
		const LightSample &light_sample, float time, Vec3 *wi, float *pdf,
		VisibilityTester *vis) const = 0;

	virtual Spectrum power(const Scene *) const = 0;
};

