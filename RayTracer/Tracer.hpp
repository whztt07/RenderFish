#pragma once
#include "Ray.hpp"
#include "Color.hpp"
#include "World.hpp"
#include "ShadeRec.hpp"
#include "DifferentialGeometry.hpp"

template<typename T>
inline Color float3_to_color(T t) {
	return Color(int(t.x * 255), int(t.y * 255), int(t.z * 255));
}

class Tracer
{
public:
	Tracer() {}
	Tracer(World* world) : _world(world) {}
	~Tracer() {}

	virtual Color trace_ray(const Ray& ray) const
	{
		return _world->intersect(ray); 
	}

protected:
	World* _world = nullptr;
};