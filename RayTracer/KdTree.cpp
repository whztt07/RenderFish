#include "KdTree.hpp"

KdTree::KdTree(const vector<Reference<Primitive>>& p, int icost, int tcost, float ebonus, int maxp, int max_depth)
	: _isect_cost(icost), _traversal_cost(tcost), _max_prims(maxp), _max_depth(max_depth), _empty_bouns() {
	for (uint32_t i = 0; i < p.size(); ++i) {
		p[i]->fully_refine(primitives);
	}
	// build Kd-tree
	next_free_node = n_alloced_nodes = 0;
	if (_max_depth <= 0)
		_max_depth = round2int(8 + 1.3f * log2int(float(primitives.size())));

	////allocate working memory
	//BoundEdge *edges[3];
	//for (int i = 0; i < 3; ++i) {
	//	edges[i] = new BoundEdge[2 * primitives.size()];
	//}

	//vector<BBox> prim_bounds;
	//// compute bounds
	//prim_bounds.reserve(primitives.size());
	//for (uint32_t i = 0; i < primitives.size(); ++i) {
	//	BBox b = primitives[i]->world_bound();
	//	bounds = combine(bounds, b);
	//	prim_bounds.push_back(b);
	//}

	//uint32_t *prim_nums = new uint32_t[primitives.size()];
	//for (uint32_t i = 0; i < primitives.size(); ++i)
	//	prim_nums[i] = i;

	//// start recursive construction
	//build_tree(0, bounds, prim_bounds, prim_nums, primitives.size(),
	//	_max_depth, edges, prims0, prims1);
}

void KdTree::build_tree(int node_num, const BBox & node_bounds, const vector<BBox>& all_prim_bounds, uint32_t * prim_nums, int n_primitives, int depth, BoundEdge * edges[3], uint32_t * prims0, uint32_t * prims1, int bad_refines) {
	//// get next free node from nodes 
	//if (next_free_node == n_alloced_nodes) {
	//	int n_alloc = std::max(2 * n_alloced_nodes, 512);
	//	KdTreeNode *n = alloc_aligned<KdTreeNode>(n_alloc);
	//	if (n_alloced_nodes > 0) {
	//		memcpy(n, nodes, n_alloced_nodes * sizeof(KdTreeNode));
	//		free_aligned(nodes);
	//	}
	//	nodes = n;
	//	n_alloced_nodes = n_alloc;
	//}
	//++next_free_node;

	//// initialize leaf node if termination criteria met
	//if (n_primitives <= _max_prims || depth == 0) {
	//	nodes[node_num].init_leaf(prim_nums, n_primitives, arena);
	//	return;
	//}
	//// initialize interior node and continue recursion
	////		choose split axis position for interior node
	////		create leaf if no good splits were found
	////		classify primitives with respect to split
	////		Recursively initialize children nodes


}
