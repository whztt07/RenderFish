#pragma once
#include "Transform.hpp"
#include "Spectrum.hpp"
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

class VisibilityTester 
{
private:
	Ray r;
public:
	void set_segment(const Point &p1, float eps1, const Point &p2, float eps2) {
		float dist = distance(p1, p2);
		r = Ray(p1, (p2 - p1) / dist, eps1, dist*(1.f - eps2));
	}
	void set_ray(const Point &p, float eps, const Vec3 &w) {
		r = Ray(p, w, eps, INFINITY);
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

	~Light() {}

	// p_w: position in world space
	virtual Spectrum sample_L(const Point &p_w, float p_epsilon, const LightSample &light_sample,
		Vec3 *wi, float *pdf, VisibilityTester *vis) const = 0;

	// total emitted power (maybe not precise)
	virtual Spectrum power(const Scene *) const = 0;

	virtual bool is_deltal_light() const = 0;

	virtual Spectrum Le(const RayDifferential & ray) const {
		return Spectrum(0.f);
	}
};

class PointLight : public Light
{
private:
	Point m_position;	// light position in world space
	Spectrum m_intensity;

public:
	PointLight(const Transform &light_to_world, const Spectrum intensity)
		: Light(light_to_world), m_position(light_to_world(Point(0, 0, 0))), m_intensity(intensity) {
	}

	virtual Spectrum sample_L(const Point &p_w, float p_epsilon, const LightSample &light_sample,
		Vec3 *wi, float *pdf, VisibilityTester *vis) const override {
		*wi = normalize(m_position - p_w);
		*pdf = 1.f;
		vis->set_segment(p_w, p_epsilon, m_position, 0.);
		return m_intensity / distance_squared(m_position, p_w);
	};

	// total emitted power (maybe not precise)
	virtual Spectrum power(const Scene *) const override {
		return 4. * M_PI * m_intensity;
	}

	virtual bool is_deltal_light() const {
		return true;
	}
};