#pragma once
#include "Ray.hpp"
#include "Color.hpp"
#include "World.hpp"
#include "ShadeRec.hpp"

class Tracer
{
public:
	Tracer() {}
	Tracer(World* world) : _world(world) {}
	~Tracer() {}

	virtual Color trace_ray(const Ray& ray) const
	{
		auto sr = _world->hit(ray);
		if (sr.hit_an_object)
			return sr.color;
		else
			return _world->background_color;
	}

protected:
	World* _world = nullptr;
};