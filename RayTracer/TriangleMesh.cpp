#include "TriangleMesh.hpp"

Triangle::Triangle(const Transform *o2w, const Transform *w2o, bool reverse_orientation, TriangleMesh *m, int n) : Shape(o2w, w2o, reverse_orientation)
{
	mesh = m;
	v = &mesh->vertex_index[3 * n];
}

BBox Triangle::object_bound() const
{
	const auto &p1 = mesh->position[v[0]];
	const auto &p2 = mesh->position[v[1]];
	const auto &p3 = mesh->position[v[2]];
	return combine(BBox((*world_to_object)(p1), (*world_to_object)(p2)), (*world_to_object)(p3));
}

BBox Triangle::world_bound() const
{
	const auto &p1 = mesh->position[v[0]];
	const auto &p2 = mesh->position[v[1]];
	const auto &p3 = mesh->position[v[2]];
	return combine(BBox(p1, p2), p3);
}
