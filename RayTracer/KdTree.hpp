#pragma once
#include "Aggregate.hpp"
#include "MemoryArena.hpp"

struct BoundEdge;
struct KdTreeNode;

class KdTree : public Aggregate
{
private:
	int _isect_cost, _traversal_cost, _max_prims, _max_depth;
	float _empty_bouns;
	vector<Reference<Primitive>> primitives;

	KdTreeNode *nodes;
	uint32_t next_free_node;
	uint32_t n_alloced_nodes;

	BBox bounds;
	MemoryArena arena;

public:

	KdTree(const vector<Reference<Primitive>> &p, int icost, int tcost,
		float ebonus, int maxp, int max_depth = 0);

	void build_tree(int node_num, const BBox &node_bounds,
		const vector<BBox> &all_prim_bounds, uint32_t *prim_nums,
		int n_primitives, int depth, BoundEdge *edges[3],
		uint32_t *prims0, uint32_t *prims1, int bad_refines = 0);

	virtual bool intersect(const Ray &ray, Intersection *isect) const override;
	virtual BBox world_bound() const override {
		return bounds;
	}
	virtual bool can_intersect() const override {
		return true;
	}
	virtual bool intersect_p(const Ray &r) const override {
		Assert(false);
		return false;
	}
};
