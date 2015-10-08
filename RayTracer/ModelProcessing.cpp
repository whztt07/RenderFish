#include "ModelProcessing.hpp"
#include <map>
using std::map;
using std::pair;
using std::make_pair;

namespace ModelProcessing {

	Mesh::Mesh(vector<Point> points, vector<int> triangles)
	{
		auto face_count = triangles.size() / 3;
		faces.resize(face_count);
		half_edges.resize(3 * face_count);
		vertices.resize(points.size());

		map<pair<int, int>, HalfEdge*> edges;

		for (int i = 0; i < face_count; i++) {
			int id[3];
			for (int j = 0; j < 3; j++) {
				id[j] = triangles[j + i];
			}
			Face* f = &faces[i];
			f->half_edge = &half_edges[i * 3];
			for (int j = 0; j < 3; j++) {
				HalfEdge* he = &half_edges[i * 3 + j];
				Vertex* v = &vertices[id[j]];
				Vertex* next_v = &vertices[id[(j + 1) % 3]];

				auto search = edges.find(make_pair(id[(j + 1) % 3], id[j]));
				if (search != edges.end()) {
					he->opposite_half_edge = search->second;
					edges.erase(search);
				}
				else {
					edges[make_pair(id[j], id[(j + 1) % 3])] = he;
				}

				if (v->outgoing_halfedge == nullptr)
					v->outgoing_halfedge = he;
				he->next_half_edge = &half_edges[i * 3 + (j + 1) % 3];
				he->face = f;
				he->to_vertex = next_v;
			}
		}
	}

	void Mesh::generate_normals(vector<Normal>* normals)
	{
		normals->clear();
		const auto count = vertices.size();
		for (int i = 0; i < count; i++) {
			auto v = vertices[i];
			auto h = v.outgoing_halfedge;
			auto h_stop = h;
			do {

				h = h->to_vertex
			} while (h != h_stop);
		}
	}
}