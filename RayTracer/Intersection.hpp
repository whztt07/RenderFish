#pragma once
#include "DifferentialGeometry.hpp"
#include "Transform.hpp"
#include "Material.hpp"
#include "MemoryArena.hpp"

class Primitive;

struct Intersection {
	DifferentialGeometry dg;
	const Primitive *primitive;
	Transform world_to_object, object_to_world;
	uint32_t shape_id, primitive_id;
	float ray_epsilon;

	BSDF* get_BSDF(const RayDifferential &ray, MemoryArena &arena) const;
};