#pragma once
#include "Transform.hpp"
#include "ReferenceCounted.h"

class Shape : public ReferenceCounted
{
public:
	const Transform *object_to_world, *world_to_object;
	const bool reverse_orientation, transform_swaps_handedness;
	const uint32_t shape_id;
	static uint32_t next_shape_id;
	
	Shape(const Transform *o2w, const Transform *w2o, bool ro)
		: object_to_world(o2w), world_to_object(w2o), reverse_orientation(ro), 
		transform_swaps_handedness(o2w->swaps_handedness()), shape_id(next_shape_id++)  {
	}
	virtual ~Shape() {};

	virtual BBox object_bound() const = 0;
	BBox world_bound() const {
		return (*object_to_world)(object_bound());
	}
};

