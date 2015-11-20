#pragma once
#include "Scene.hpp"
#include "Camera.hpp"
#include "Renderer.hpp"

class Integrator
{
public:
	Integrator() {};

	virtual void preprocess(const Scene *scene, const Camera *camera, const Renderer *renderer) {};
	virtual void request_samples(Sampler *sampler, Sample *sample, const Scene* scene) {};

};


class SurfaceIntegrator : public Integrator {
public:
	virtual Spectrum Li(const Scene *scene, const Renderer *renderer, const RayDifferential &ray,
		const Intersection &isect, const Sample *sample, RNG &rng, MemoryArena &arena) const = 0;
};

class VolumeIntegrator : public Integrator {
	virtual Spectrum Li(const Scene *scene, const Renderer *renderer, const RayDifferential &ray,
		const Intersection &isect, const Sample *sample, RNG &rng, MemoryArena &arena) const = 0;
};

class DepthIntegrator : public SurfaceIntegrator {
private:
	Spectrum m_color;
	float m_max_dist = -std::numeric_limits<float>::infinity();

public:

	DepthIntegrator() : SurfaceIntegrator(), m_color(Spectrum(1.0f)) {

	}

	virtual void preprocess(const Scene *scene, const Camera *camera, const Renderer *renderer) {
		SurfaceIntegrator::preprocess(scene, camera, renderer);
		const auto& scene_aabb = scene->world_bound();
		Point camera_position = camera->camera_to_world(Point(0, 0, 0));
		Point corners[8];
		corners[0] = scene_aabb.pmin;
		corners[7] = scene_aabb.pmax;
		Vec3 bbox_dir = scene_aabb.pmax - scene_aabb.pmin;
		corners[1] = corners[0] + Vec3(bbox_dir.x, 0, 0);
		corners[2] = corners[0] + Vec3(0, bbox_dir.y, 0);
		corners[3] = corners[0] + Vec3(0, 0, bbox_dir.z);
		corners[4] = corners[0] + Vec3(bbox_dir.x, bbox_dir.y, 0);
		corners[5] = corners[0] + Vec3(bbox_dir.x, 0, bbox_dir.z);
		corners[6] = corners[0] + Vec3(0, bbox_dir.x, bbox_dir.z);
		for (int i = 0; i < 8; ++i)
			m_max_dist = max(m_max_dist, (camera_position - corners[i]).length());
		info("max distance in NormalIntefrator\n", m_max_dist);
	}

	virtual Spectrum Li(const Scene *scene, const Renderer *renderer, const RayDifferential &ray,
		const Intersection &isect, const Sample *sample, RNG &rng, MemoryArena &arena) const override {
		
		float distance = ray.maxt;
		return Spectrum(1.0f - distance / m_max_dist) * m_color;
	}
};