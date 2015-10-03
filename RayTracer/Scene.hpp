#pragma once
#include "Aggregate.hpp"
#include "Ray.hpp"
#include "Intersection.hpp"

class Scene
{
public:
	Aggregate *aggregate;
	//vector<Light *> light;
	//VolumeRegion *volumeRegion;
	BBox bound;

public:
	Scene();
	~Scene();

	bool intersect(const Ray &ray, Intersection *isect) const {
		return aggregate->intersect(ray, isect);
	}

	bool intersect_p(const Ray& ray) const {
		return aggregate->intersect_p(ray);
	}

	const BBox & world_bound() const {
		return bound;
	}
};

