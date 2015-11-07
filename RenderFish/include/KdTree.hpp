#pragma once
#include "Aggregate.hpp"
#include "MemoryArena.hpp"

//#define USE_PBRT_KDTREE

#if 0
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

#else

// KdTreeAccel Declarations
struct KdAccelNode;
struct BoundEdge;
class KdTreeAccel : public Aggregate {
public:
	// KdTreeAccel Public Methods
	KdTreeAccel(const vector<Reference<Primitive> > &p,
		int icost = 80, int scost = 1, float ebonus = 0.5f, int maxp = 1,
		int maxDepth = -1);
	virtual BBox world_bound() const { return bounds; }
	virtual bool can_intersect() const { return true; }
	~KdTreeAccel();
	virtual bool intersect(const Ray &ray, Intersection *isect) const;
	virtual bool intersect_p(const Ray &ray) const;
private:
	// KdTreeAccel Private Methods
	void buildTree(int nodeNum, const BBox &bounds,
		const vector<BBox> &primBounds, uint32_t *primNums, int nprims, int depth,
		BoundEdge *edges[3], uint32_t *prims0, uint32_t *prims1, int badRefines = 0);

	// KdTreeAccel Private Data
	int isectCost, traversalCost, maxPrims, maxDepth;
	float emptyBonus;
	vector<Reference<Primitive> > primitives;
	KdAccelNode *nodes;
	int nAllocedNodes, nextFreeNode;
	BBox bounds;
	MemoryArena arena;
};


struct KdToDo {
	const KdAccelNode *node;
	float tmin, tmax;
};

typedef KdTreeAccel KdTree;

#endif