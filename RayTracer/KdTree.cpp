#include "KdTree.hpp"
#include "MathHelper.hpp"
#include "Timer.hpp"

#if 1
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
	
	log_group("Start building Kd-Tree");
	Timer timer("Build Kd-Tree");
	timer.begin();

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

	timer.end();
	info("Finished!\n");
	timer.print();
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

#else

typedef Vec3 Vector;

// KdTreeAccel Local Declarations
struct KdAccelNode {
	// KdAccelNode Methods
	void initLeaf(uint32_t *primNums, int np, MemoryArena &arena);
	void initInterior(uint32_t axis, uint32_t ac, float s) {
		split = s;
		flags = axis;
		aboveChild |= (ac << 2);
	}
	float SplitPos() const { return split; }
	uint32_t nPrimitives() const { return nPrims >> 2; }
	uint32_t SplitAxis() const { return flags & 3; }
	bool IsLeaf() const { return (flags & 3) == 3; }
	uint32_t AboveChild() const { return aboveChild >> 2; }
	union {
		float split;            // Interior
		uint32_t onePrimitive;  // Leaf
		uint32_t *primitives;   // Leaf
	};

private:
	union {
		uint32_t flags;         // Both
		uint32_t nPrims;        // Leaf
		uint32_t aboveChild;    // Interior
	};
};


struct BoundEdge {
	// BoundEdge Public Methods
	BoundEdge() { }
	BoundEdge(float tt, int pn, bool starting) {
		t = tt;
		primNum = pn;
		type = starting ? START : END;
	}
	bool operator<(const BoundEdge &e) const {
		if (t == e.t)
			return (int)type < (int)e.type;
		else return t < e.t;
	}
	float t;
	int primNum;
	enum { START, END } type;
};



// KdTreeAccel Method Definitions
KdTreeAccel::KdTreeAccel(const vector<Reference<Primitive> > &p,
	int icost, int tcost, float ebonus, int maxp,
	int md)
	: isectCost(icost), traversalCost(tcost), maxPrims(maxp), maxDepth(md),
	emptyBonus(ebonus) {
	//PBRT_KDTREE_STARTED_CONSTRUCTION(this, p.size());
	for (uint32_t i = 0; i < p.size(); ++i)
		p[i]->fully_refine(primitives);
	// Build kd-tree for accelerator
	nextFreeNode = nAllocedNodes = 0;
	if (maxDepth <= 0)
		maxDepth = round2int(8 + 1.3f * log2int(float(primitives.size())));

	// Compute bounds for kd-tree construction
	vector<BBox> primBounds;
	primBounds.reserve(primitives.size());
	for (uint32_t i = 0; i < primitives.size(); ++i) {
		BBox b = primitives[i]->world_bound();
		bounds = combine(bounds, b);
		primBounds.push_back(b);
	}

	// Allocate working memory for kd-tree construction
	BoundEdge *edges[3];
	for (int i = 0; i < 3; ++i)
		edges[i] = new BoundEdge[2 * primitives.size()];
	uint32_t *prims0 = new uint32_t[primitives.size()];
	uint32_t *prims1 = new uint32_t[(maxDepth + 1) * primitives.size()];

	// Initialize _primNums_ for kd-tree construction
	uint32_t *primNums = new uint32_t[primitives.size()];
	for (uint32_t i = 0; i < primitives.size(); ++i)
		primNums[i] = i;

	// Start recursive construction of kd-tree
	buildTree(0, bounds, primBounds, primNums, primitives.size(),
		maxDepth, edges, prims0, prims1);

	// Free working memory for kd-tree construction
	delete[] primNums;
	for (int i = 0; i < 3; ++i)
		delete[] edges[i];
	delete[] prims0;
	delete[] prims1;
	//PBRT_KDTREE_FINISHED_CONSTRUCTION(this);
}


void KdAccelNode::initLeaf(uint32_t *primNums, int np,
	MemoryArena &arena) {
	flags = 3;
	nPrims |= (np << 2);
	// Store primitive ids for leaf node
	if (np == 0)
		onePrimitive = 0;
	else if (np == 1)
		onePrimitive = primNums[0];
	else {
		primitives = arena.alloc<uint32_t>(np);
		for (int i = 0; i < np; ++i)
			primitives[i] = primNums[i];
	}
}


KdTreeAccel::~KdTreeAccel() {
	free_aligned(nodes);
	//FreeAligned(nodes);
}


void KdTreeAccel::buildTree(int nodeNum, const BBox &nodeBounds,
	const vector<BBox> &allPrimBounds, uint32_t *primNums,
	int nPrimitives, int depth, BoundEdge *edges[3],
	uint32_t *prims0, uint32_t *prims1, int badRefines) {
	Assert(nodeNum == nextFreeNode);
	// Get next free node from _nodes_ array
	if (nextFreeNode == nAllocedNodes) {
		int nAlloc = max(2 * nAllocedNodes, 512);
		KdAccelNode *n = alloc_aligned<KdAccelNode>(nAlloc);
		if (nAllocedNodes > 0) {
			memcpy(n, nodes, nAllocedNodes * sizeof(KdAccelNode));
			//FreeAligned(nodes);
			free_aligned(nodes);
		}
		nodes = n;
		nAllocedNodes = nAlloc;
	}
	++nextFreeNode;

	// Initialize leaf node if termination criteria met
	if (nPrimitives <= maxPrims || depth == 0) {
		//PBRT_KDTREE_CREATED_LEAF(nPrimitives, maxDepth - depth);
		nodes[nodeNum].initLeaf(primNums, nPrimitives, arena);
		return;
	}

	// Initialize interior node and continue recursion

	// Choose split axis position for interior node
	int bestAxis = -1, bestOffset = -1;
	float bestCost = INFINITY;
	float oldCost = isectCost * float(nPrimitives);
	float totalSA = nodeBounds.surface_area();
	float invTotalSA = 1.f / totalSA;
	Vector d = nodeBounds.pmax - nodeBounds.pmin;

	// Choose which axis to split along
	uint32_t axis = nodeBounds.maximum_extent();
	int retries = 0;
retrySplit:

	// Initialize edges for _axis_
	for (int i = 0; i < nPrimitives; ++i) {
		int pn = primNums[i];
		const BBox &bbox = allPrimBounds[pn];
		edges[axis][2 * i] = BoundEdge(bbox.pmin[axis], pn, true);
		edges[axis][2 * i + 1] = BoundEdge(bbox.pmax[axis], pn, false);
	}
	std::sort(&edges[axis][0], &edges[axis][2 * nPrimitives]);

	// Compute cost of all splits for _axis_ to find best
	int nBelow = 0, nAbove = nPrimitives;
	for (int i = 0; i < 2 * nPrimitives; ++i) {
		if (edges[axis][i].type == BoundEdge::END) --nAbove;
		float edget = edges[axis][i].t;
		if (edget > nodeBounds.pmin[axis] &&
			edget < nodeBounds.pmax[axis]) {
			// Compute cost for split at _i_th edge
			uint32_t otherAxis0 = (axis + 1) % 3, otherAxis1 = (axis + 2) % 3;
			float belowSA = 2 * (d[otherAxis0] * d[otherAxis1] +
				(edget - nodeBounds.pmin[axis]) *
				(d[otherAxis0] + d[otherAxis1]));
			float aboveSA = 2 * (d[otherAxis0] * d[otherAxis1] +
				(nodeBounds.pmax[axis] - edget) *
				(d[otherAxis0] + d[otherAxis1]));
			float pBelow = belowSA * invTotalSA;
			float pAbove = aboveSA * invTotalSA;
			float eb = (nAbove == 0 || nBelow == 0) ? emptyBonus : 0.f;
			float cost = traversalCost +
				isectCost * (1.f - eb) * (pBelow * nBelow + pAbove * nAbove);

			// Update best split if this is lowest cost so far
			if (cost < bestCost) {
				bestCost = cost;
				bestAxis = axis;
				bestOffset = i;
			}
		}
		if (edges[axis][i].type == BoundEdge::START) ++nBelow;
	}
	Assert(nBelow == nPrimitives && nAbove == 0);

	// Create leaf if no good splits were found
	if (bestAxis == -1 && retries < 2) {
		++retries;
		axis = (axis + 1) % 3;
		goto retrySplit;
	}
	if (bestCost > oldCost) ++badRefines;
	if ((bestCost > 4.f * oldCost && nPrimitives < 16) ||
		bestAxis == -1 || badRefines == 3) {
		//PBRT_KDTREE_CREATED_LEAF(nPrimitives, maxDepth - depth);
		nodes[nodeNum].initLeaf(primNums, nPrimitives, arena);
		return;
	}

	// Classify primitives with respect to split
	int n0 = 0, n1 = 0;
	for (int i = 0; i < bestOffset; ++i)
		if (edges[bestAxis][i].type == BoundEdge::START)
			prims0[n0++] = edges[bestAxis][i].primNum;
	for (int i = bestOffset + 1; i < 2 * nPrimitives; ++i)
		if (edges[bestAxis][i].type == BoundEdge::END)
			prims1[n1++] = edges[bestAxis][i].primNum;

	// Recursively initialize children nodes
	float tsplit = edges[bestAxis][bestOffset].t;
	//PBRT_KDTREE_CREATED_INTERIOR_NODE(bestAxis, tsplit);
	BBox bounds0 = nodeBounds, bounds1 = nodeBounds;
	bounds0.pmax[bestAxis] = bounds1.pmin[bestAxis] = tsplit;
	buildTree(nodeNum + 1, bounds0,
		allPrimBounds, prims0, n0, depth - 1, edges,
		prims0, prims1 + nPrimitives, badRefines);
	uint32_t aboveChild = nextFreeNode;
	nodes[nodeNum].initInterior(bestAxis, aboveChild, tsplit);
	buildTree(aboveChild, bounds1, allPrimBounds, prims1, n1,
		depth - 1, edges, prims0, prims1 + nPrimitives, badRefines);
}


bool KdTreeAccel::intersect(const Ray &ray,
	Intersection *isect) const {
	//PBRT_KDTREE_INTERSECTION_TEST(const_cast<KdTreeAccel *>(this), const_cast<Ray *>(&ray));
	// Compute initial parametric range of ray inside kd-tree extent
	float tmin, tmax;
	if (!bounds.intersect_p(ray, &tmin, &tmax))
	{
		//PBRT_KDTREE_RAY_MISSED_BOUNDS();
		return false;
	}

	// Prepare to traverse kd-tree for ray
	Vector invDir(1.f / ray.d.x, 1.f / ray.d.y, 1.f / ray.d.z);
#define MAX_TODO 64
	KdToDo todo[MAX_TODO];
	int todoPos = 0;

	// Traverse kd-tree nodes in order for ray
	bool hit = false;
	const KdAccelNode *node = &nodes[0];
	while (node != NULL) {
		// Bail out if we found a hit closer than the current node
		if (ray.maxt < tmin) break;
		if (!node->IsLeaf()) {
			//PBRT_KDTREE_INTERSECTION_TRAVERSED_INTERIOR_NODE(const_cast<KdAccelNode *>(node));
			// Process kd-tree interior node

			// Compute parametric distance along ray to split plane
			int axis = node->SplitAxis();
			float tplane = (node->SplitPos() - ray.o[axis]) * invDir[axis];

			// Get node children pointers for ray
			const KdAccelNode *firstChild, *secondChild;
			int belowFirst = (ray.o[axis] <  node->SplitPos()) ||
				(ray.o[axis] == node->SplitPos() && ray.d[axis] <= 0);
			if (belowFirst) {
				firstChild = node + 1;
				secondChild = &nodes[node->AboveChild()];
			}
			else {
				firstChild = &nodes[node->AboveChild()];
				secondChild = node + 1;
			}

			// Advance to next child node, possibly enqueue other child
			if (tplane > tmax || tplane <= 0)
				node = firstChild;
			else if (tplane < tmin)
				node = secondChild;
			else {
				// Enqueue _secondChild_ in todo list
				todo[todoPos].node = secondChild;
				todo[todoPos].tmin = tplane;
				todo[todoPos].tmax = tmax;
				++todoPos;
				node = firstChild;
				tmax = tplane;
			}
		}
		else {
			//PBRT_KDTREE_INTERSECTION_TRAVERSED_LEAF_NODE(const_cast<KdAccelNode *>(node), node->nPrimitives());
			// Check for intersections inside leaf node
			uint32_t nPrimitives = node->nPrimitives();
			if (nPrimitives == 1) {
				const Reference<Primitive> &prim = primitives[node->onePrimitive];
				// Check one primitive inside leaf node
				//PBRT_KDTREE_INTERSECTION_PRIMITIVE_TEST(const_cast<Primitive *>(prim.GetPtr()));
				if (prim->intersect(ray, isect))
				{
					//PBRT_KDTREE_INTERSECTION_HIT(const_cast<Primitive *>(prim.GetPtr()));
					hit = true;
				}
			}
			else {
				uint32_t *prims = node->primitives;
				for (uint32_t i = 0; i < nPrimitives; ++i) {
					const Reference<Primitive> &prim = primitives[prims[i]];
					// Check one primitive inside leaf node
					//PBRT_KDTREE_INTERSECTION_PRIMITIVE_TEST(const_cast<Primitive *>(prim.GetPtr()));
					if (prim->intersect(ray, isect))
					{
						//PBRT_KDTREE_INTERSECTION_HIT(const_cast<Primitive *>(prim.GetPtr()));
						hit = true;
					}
				}
			}

			// Grab next node to process from todo list
			if (todoPos > 0) {
				--todoPos;
				node = todo[todoPos].node;
				tmin = todo[todoPos].tmin;
				tmax = todo[todoPos].tmax;
			}
			else
				break;
		}
	}
	//PBRT_KDTREE_INTERSECTION_FINISHED();
	return hit;
}


bool KdTreeAccel::intersect_p(const Ray &ray) const {
	//PBRT_KDTREE_INTERSECTIONP_TEST(const_cast<KdTreeAccel *>(this), const_cast<Ray *>(&ray));
	// Compute initial parametric range of ray inside kd-tree extent
	float tmin, tmax;
	if (!bounds.intersect_p(ray, &tmin, &tmax))
	{
		//PBRT_KDTREE_RAY_MISSED_BOUNDS();
		return false;
	}

	// Prepare to traverse kd-tree for ray
	Vector invDir(1.f / ray.d.x, 1.f / ray.d.y, 1.f / ray.d.z);
#define MAX_TODO 64
	KdToDo todo[MAX_TODO];
	int todoPos = 0;
	const KdAccelNode *node = &nodes[0];
	while (node != NULL) {
		if (node->IsLeaf()) {
			//PBRT_KDTREE_INTERSECTIONP_TRAVERSED_LEAF_NODE(const_cast<KdAccelNode *>(node), node->nPrimitives());
			// Check for shadow ray intersections inside leaf node
			uint32_t nPrimitives = node->nPrimitives();
			if (nPrimitives == 1) {
				const Reference<Primitive> &prim = primitives[node->onePrimitive];
				//PBRT_KDTREE_INTERSECTIONP_PRIMITIVE_TEST(const_cast<Primitive *>(prim.GetPtr()));
				if (prim->intersect_p(ray)) {
					//PBRT_KDTREE_INTERSECTIONP_HIT(const_cast<Primitive *>(prim.GetPtr()));
					return true;
				}
			}
			else {
				uint32_t *prims = node->primitives;
				for (uint32_t i = 0; i < nPrimitives; ++i) {
					const Reference<Primitive> &prim = primitives[prims[i]];
					//PBRT_KDTREE_INTERSECTIONP_PRIMITIVE_TEST(const_cast<Primitive *>(prim.GetPtr()));
					if (prim->intersect_p(ray)) {
						//PBRT_KDTREE_INTERSECTIONP_HIT(const_cast<Primitive *>(prim.GetPtr()));
						return true;
					}
				}
			}

			// Grab next node to process from todo list
			if (todoPos > 0) {
				--todoPos;
				node = todo[todoPos].node;
				tmin = todo[todoPos].tmin;
				tmax = todo[todoPos].tmax;
			}
			else
				break;
		}
		else {
			//PBRT_KDTREE_INTERSECTIONP_TRAVERSED_INTERIOR_NODE(const_cast<KdAccelNode *>(node));
			// Process kd-tree interior node

			// Compute parametric distance along ray to split plane
			int axis = node->SplitAxis();
			float tplane = (node->SplitPos() - ray.o[axis]) * invDir[axis];

			// Get node children pointers for ray
			const KdAccelNode *firstChild, *secondChild;
			int belowFirst = (ray.o[axis] <  node->SplitPos()) ||
				(ray.o[axis] == node->SplitPos() && ray.d[axis] <= 0);
			if (belowFirst) {
				firstChild = node + 1;
				secondChild = &nodes[node->AboveChild()];
			}
			else {
				firstChild = &nodes[node->AboveChild()];
				secondChild = node + 1;
			}

			// Advance to next child node, possibly enqueue other child
			if (tplane > tmax || tplane <= 0)
				node = firstChild;
			else if (tplane < tmin)
				node = secondChild;
			else {
				// Enqueue _secondChild_ in todo list
				todo[todoPos].node = secondChild;
				todo[todoPos].tmin = tplane;
				todo[todoPos].tmax = tmax;
				++todoPos;
				node = firstChild;
				tmax = tplane;
			}
		}
	}
	//PBRT_KDTREE_INTERSECTIONP_MISSED();
	return false;
}

#endif