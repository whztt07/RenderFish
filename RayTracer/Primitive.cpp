#include "Primitive.hpp"

uint32_t Primitive::next_primitive_id = 1;

void Primitive::fully_refine(vector<Reference<Primitive>>& refined) const
{
	vector < Reference<Primitive> > todo;
	todo.push_back(const_cast<Primitive*>(this));
	while (todo.size()) {
		auto prim = todo.back();
		todo.pop_back();
		if (prim->can_intersect())
			refined.push_back(prim);
		else
			prim->refine(todo);
	}
}

inline bool GeometryPrimitive::intersect(const Ray & r, Intersection * isect) const {
	float thit, ray_epsilon;
	if (!_shape->intersect(r, &thit, &ray_epsilon, &isect->dg))
		return false;
	isect->primitive = this;
	isect->world_to_object = *_shape->world_to_object;
	isect->object_to_world = *_shape->object_to_world;
	isect->shape_id = _shape->shape_id;
	isect->primitive_id = primitive_id;
	isect->ray_epsilon = ray_epsilon;
	r.maxt = thit;
	return true;
}
