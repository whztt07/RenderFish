#include "KdTree.hpp"
#include "MathHelper.hpp"


struct KdTreeNode {
public:
	union {
		float	 split;			// interior, the postion of the splitting plane along the axis
		uint32_t one_primitive;	// leaf
		uint32_t *primitives;	// leaf
	};

private:
	union {
		uint32_t _flags;		// both, use 2 low-order bits, 0-x, 1-y, 2-z, 3-leaf node
		uint32_t _n_prims;		// leaf
		uint32_t _above_child;	// interior
	};

public:
	void init_leaf(uint32_t *prim_nums, int np, MemoryArena &arena) {
		_flags = 3;
		_n_prims |= (np << 2);

		// store primitive ids for leaf node
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
		_flags = axis;
		_above_child |= (ac << 2);
	}

	bool	is_leaf()	const { return (_flags & 3) == 3; }
	float	split_pos() const { return split; }
	uint32_t n_primitives() const { return _n_prims >> 2; }
	uint32_t split_axis()	const { return _flags & 3; }
	uint32_t above_child()	const { return _above_child >> 2; }

};

struct BoundEdge {
	float t;
	int prim_num;
	enum { START, END } type;

	BoundEdge() {}
	BoundEdge(float t, int pn, bool starting) : t(t), prim_num(pn) {
		type = starting ? START : END;
	}

	bool operator<(const BoundEdge &e) const {
		if (t == e.t)
			return (int)type < (int)e.type;
		else
			return t < e.t;
	}
};

struct KdToDo {
	const KdTreeNode *node;
	float tmin, tmax;
};

KdTree::KdTree(const vector<Reference<Primitive>>& p, int icost, int tcost, float ebonus, int maxp, int max_depth)
	: _isect_cost(icost), _traversal_cost(tcost), _max_prims(maxp), _max_depth(max_depth), _empty_bouns() {
	
	info("Start building Kd-Tree.\n");

	for (uint32_t i = 0; i < p.size(); ++i) {
		p[i]->fully_refine(primitives);
	}
	// build Kd-tree for accelerator 232
	next_free_node = n_alloced_nodes = 0;
	if (max_depth <= 0)
		_max_depth = round2int(8 + 1.3f * log2int(float(primitives.size())));

	// compute bounds for kd-tree construction 232
	vector<BBox> prim_bounds;
	prim_bounds.reserve(primitives.size());
	for (uint32_t i = 0; i < primitives.size(); ++i) {
		BBox b = primitives[i]->world_bound();
		bounds = combine(bounds, b);
		prim_bounds.push_back(b);
	}

	//allocate working memory 236
	BoundEdge *edges[3];
	for (int i = 0; i < 3; ++i) {
		edges[i] = new BoundEdge[2 * primitives.size()];
	}
	uint32_t *prims0 = new uint32_t[primitives.size()];
	uint32_t *prims1 = new uint32_t[(_max_depth + 1) * primitives.size()];

	// initialize prim_nums for kd-tree construction
	uint32_t *prim_nums = new uint32_t[primitives.size()];
	for (uint32_t i = 0; i < primitives.size(); ++i)
		prim_nums[i] = i;

	// start recursive construction
	build_tree(0, bounds, prim_bounds, prim_nums, primitives.size(),
		_max_depth, edges, prims0, prims1, 0);

	// free working memory for kd-tree construction
	delete[] edges[0];
	delete[] edges[1];
	delete[] edges[2];
	delete[] prims0;
	delete[] prims1;
	delete[] prim_nums;

	info("Finished!\n");
}

void KdTree::build_tree(int node_num, const BBox & node_bounds, const vector<BBox>& all_prim_bounds,
	uint32_t * prim_nums, int n_primitives, int depth, BoundEdge * edges[3],
	uint32_t * prims0, uint32_t * prims1, int bad_refines) {

	// get next free node from nodes 
	if (next_free_node == n_alloced_nodes) {
		int n_alloc = max<uint32_t>(2 * n_alloced_nodes, 512);
		KdTreeNode *n = alloc_aligned<KdTreeNode>(n_alloc);
		if (n_alloced_nodes > 0) {
			memcpy(n, nodes, n_alloced_nodes * sizeof(KdTreeNode));
			free_aligned(nodes);
		}
		nodes = n;
		n_alloced_nodes = n_alloc;
	}
	++next_free_node;

	// initialize leaf node if termination criteria met
	if (n_primitives <= _max_prims || depth == 0) {
		nodes[node_num].init_leaf(prim_nums, n_primitives, arena);
		return;
	}
	// initialize interior node and continue recursion
	//		choose split axis position for interior node 236
	int best_axis = -1, best_offset = -1;
	float best_cost = INFINITY;
	float old_cost = _isect_cost * float(n_primitives);
	float total_SA = node_bounds.surface_area();
	float inv_total_SA = 1.f / total_SA;
	Vec3 d = node_bounds.pmax - node_bounds.pmin;
	//			choose which axis to split along 236
	uint32_t axis = node_bounds.maximum_extent();
	int retries = 0;
retry_split:
	//			initialize edges for axis 236
	for (int i = 0; i < n_primitives; ++i) {
		int pn = prim_nums[i];
		const BBox &bbox = all_prim_bounds[pn];
		edges[axis][2 * i] = BoundEdge(bbox.pmin[axis], pn, true);
		edges[axis][2 * i + 1] = BoundEdge(bbox.pmax[axis], pn, false);
	}
	std::sort(&edges[axis][0], &edges[axis][2 * n_primitives]);
	//			compute cost of all split for axis to find best 237
	int n_below = 0, n_above = n_primitives;
	for (int i = 0; i < 2 * n_primitives; ++i) {
		if (edges[axis][i].type == BoundEdge::END)
			--n_above;
		float edget = edges[axis][i].t;
		if (edget > node_bounds.pmin[axis] && edget < node_bounds.pmax[axis]) {
			// compute cost for split at ith edge
			uint32_t other_axis_0 = (axis + 1) % 3, other_axis_1 = (axis + 2) % 3;
			float below_SA = 2 * (d[other_axis_0] * d[other_axis_1] +
				(edget - node_bounds.pmin[axis]) * (d[other_axis_0] + d[other_axis_1]));
			float above_SA = 2 * (d[other_axis_0] * d[other_axis_1] +
				(node_bounds.pmax[axis] - edget) * (d[other_axis_0] + d[other_axis_1]));
			float p_below = below_SA * inv_total_SA;
			float p_above = above_SA * inv_total_SA;
			float eb = (n_above == 0 || n_below == 0) ? _empty_bouns : 0.f;
			float cost = _traversal_cost + _isect_cost * (1.f - eb) * (p_below * n_below + p_above * n_above);
			// update best split if this is lowest cost so far
			if (cost < best_cost) {
				best_cost = cost;
				best_axis = axis;
				best_offset = i;
			}

		}
		if (edges[axis][i].type == BoundEdge::START)
			++n_below;
	}
	//		create leaf if no good splits were found 239
	if (best_axis == -1 && retries < 2) {
		++retries;
		axis = (axis + 1) % 3;
		goto retry_split;
	}
	if (best_cost > old_cost)
		++bad_refines;
	if ((best_cost > 4.f * old_cost && n_primitives < 16) ||
		best_axis == -1 || bad_refines == 3) {
		nodes[node_num].init_leaf(prim_nums, n_primitives, arena);
		return;
	}
	//		classify primitives with respect to split 239
	int n0 = 0, n1 = 0;
	for (int i = 0; i < best_offset; ++i) {
		if (edges[best_axis][i].type == BoundEdge::START)
			prims0[n0++] = edges[best_axis][i].prim_num;
	}
	for (int i = best_offset + 1; i < 2 * n_primitives; ++i) {
		if (edges[best_axis][i].type == BoundEdge::END)
			prims1[n1++] = edges[best_axis][i].prim_num;
	}
	//		Recursively initialize children nodes 240
	float tsplit = edges[best_axis][best_offset].t;
	BBox bounds0 = node_bounds, bounds1 = node_bounds;
	bounds0.pmax[best_axis] = bounds1.pmin[best_axis] = tsplit;
	build_tree(node_num + 1, bounds0, all_prim_bounds, prims0,
		n0, depth - 1, edges, prims0, prims1 + n_primitives, bad_refines);
	uint32_t above_child = next_free_node;
	nodes[node_num].init_interior(best_axis, above_child, tsplit);
	build_tree(above_child, bounds1, all_prim_bounds, prims1, n1,
		depth - 1, edges, prims0, prims1 + n_primitives, bad_refines);
}

inline bool KdTree::intersect(const Ray & ray, Intersection * isect) const {
	// compute initial parametric range of ray inside kd-tree extent
	float tmin, tmax;
	if (!bounds.intersect_p(ray, &tmin, &tmax))
		return false;
	// prepare to traverse kd-tree for ray
	Vec3 inv_dir(1.f / ray.d.x, 1.f / ray.d.y, 1.f / ray.d.z);
#define MAX_TODO 64
	KdToDo todo[MAX_TODO];
	int todo_pos = 0;
	// traverse kd-tree nodes in order for ray
	bool hit = false;
	const KdTreeNode *node = &nodes[0];
	while (node != nullptr) {
		// bail out if we found a hit closer than the current node
		if (ray.maxt < tmin)
			break;
		if (!node->is_leaf()) {
			// process kd-tree interior node
			//		compute parametric distance along ray to split plane
			int axis = node->split_axis();
			float tplane = (node->split_pos() - ray.o[axis]) * inv_dir[axis];
			//		get node children pointers for ray
			const KdTreeNode *first_child, *second_child;
			int below_first = (ray.o[axis] < node->split_pos()) ||
				(ray.o[axis] == node->split_pos() && ray.d[axis] <= 0);
			if (below_first) {
				first_child = node + 1;
				second_child = &nodes[node->above_child()];
			}
			else {
				first_child = &nodes[node->above_child()];
				second_child = node + 1;
			}
			//		advance to next child node, possibly enqueue other child
			if (tplane > tmax || tplane <= 0)
				node = first_child;
			else if (tplane < tmin)
				node = second_child;
			else {
				// enqueue second_child in todo list
				todo[todo_pos].node = second_child;
				todo[todo_pos].tmin = tplane;
				todo[todo_pos].tmax = tmax;
				++todo_pos;
				node = first_child;
				tmax = tplane;
			}
		}
		else {
			// check for intersections inside leaf node
			uint32_t n_primitives = node->n_primitives();
			if (n_primitives == 1) {
				const auto &prim = primitives[node->one_primitive];
				// check one primitive inside leaf node
				if (prim->intersect(ray, isect))
					hit = true;
			}
			else {
				uint32_t *prims = node->primitives;
				for (uint32_t i = 0; i < n_primitives; i++) {
					const auto &prim = primitives[prims[i]];
					// check one primitive inside leaf node
					if (prim->intersect(ray, isect))
						hit = true;
				}
			}
			// grab next node to process from todo list
			if (todo_pos <= 0) {
				break;
			}
			--todo_pos;
			node = todo[todo_pos].node;
			tmin = todo[todo_pos].tmin;
			tmax = todo[todo_pos].tmax;
		}
	}
	return hit;
}
