#pragma once
#include "DifferentialGeometry.hpp"
#include "Transform.hpp"

class Primitive;

struct Intersection {
	DifferentialGeometry dg;
	const Primitive *primitive;
	Transform world_to_object, object_to_world;
	uint32_t shape_id, primitive_id;
	float ray_epsilon;
};