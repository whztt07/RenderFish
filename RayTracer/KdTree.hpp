#pragma once
#include "Aggregate.hpp"
#include "MemoryArena.hpp"

struct KdTreeNode {
private:
	union {
		float split;			// interior, the postion of the splitting plane along the axis
		uint32_t one_primitive; // leaf
		uint32_t *primitives;	// leaf
	};

	union {
		uint32_t flags;			// both, use 2 low-order bits, 0-x, 1-y, 2-z, 3-leaf node
		uint32_t n_prims;		// leaf
		uint32_t above_child;	// interior
	};

public:
	void init_leaf(uint32_t *prim_nums, int np, MemoryArena &arena) {
		flags = 3;
		n_prims |= (np << 2);
		if (np == 0)
			one_primitive = 0;
		else if (np == 1)
			one_primitive = prim_nums[0];
		else {
			primitives = arena.alloc<uint32_t>(np);
			for (int i = 0; i < np; i++)
				primitives[i] = prim_nums[i];
		}
	}

	void init_interior(uint32_t axis, uint32_t ac, float s) {
		split = s;
		flags = axis;
		above_child = (ac << 2);
	}

	bool	is_leaf()	const { return (flags & 3) == 3; }
	float	split_pos() const { return split; }
	uint32_t n_primitives() const { return n_prims >> 2; }
	uint32_t split_axis()	const { return flags & 3; }
	uint32_t above_all()	const { return above_child >> 2; }

};

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

	struct BoundEdge {
		float t;
		int prim_num;
		enum {START, END } type;

		BoundEdge(float t, int pn, bool starting) : t(t), prim_num(pn) {
			type = starting ? START : END;
		}
	};

	KdTree(const vector<Reference<Primitive>> &p, int icost, int tcost,
		float ebonus, int maxp, int max_depth = 0);

	void build_tree(int node_num, const BBox &node_bounds,
		const vector<BBox> &all_prim_bounds, uint32_t *prim_nums,
		int n_primitives, int depth, BoundEdge *edges[3],
		uint32_t *prims0, uint32_t *prims1, int bad_refines);
};

