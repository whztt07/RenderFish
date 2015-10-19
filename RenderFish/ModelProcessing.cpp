#include "ModelProcessing.hpp"
#include <map>
using std::map;
using std::pair;
using std::make_pair;

namespace ModelProcessing {

	Mesh::Mesh(vector<Point> points, vector<int> triangles) : positions(points), indices(triangles)
	{
		auto face_count = triangles.size() / 3;
		faces.resize(face_count);
		half_edges.resize(3 * face_count);
		vertices.resize(points.size());

		map<pair<int, int>, HalfEdge*> edges;

		for (int i = 0; i < face_count; i++) {

			Face* f = &faces[i];
			f->id = i;
			f->half_edge = &half_edges[i * 3];
			for (int j = 0; j < 3; j++) {
				int id1 = triangles[j];
				int id2 = triangles[(j + 1) % 3];
				HalfEdge* he = &half_edges[i * 3 + j];
				Vertex* v = &vertices[id1];
				Vertex* next_v = &vertices[id2];

				auto search = edges.find(make_pair(id2, id1));
				if (search != edges.end()) {
					auto o_he = search->second;
					he->opposite_half_edge = o_he;
					o_he->opposite_half_edge = he;
					edges.erase(search);
				}
				else {
					edges[make_pair(id1, id2)] = he;
				}

				if (v->outgoing_halfedge == nullptr)
					v->outgoing_halfedge = he;
				he->next_half_edge = &half_edges[i * 3 + (j + 1) % 3];
				he->face = f;
				he->to_vertex = next_v;
			}
		}

		info("number of bound edges: %d\n", edges.size());
		//Assert(edges.size() == 0);
	}

	void Mesh::generate_normals(vector<Normal>* normals)
	{
		normals->clear();
		normals->reserve(positions.size());
		vector<Vec3> face_normals;
		face_normals.reserve(faces.size());

		for (int i = 0; i < faces.size(); ++i) {
			auto p0 = positions[indices[i * 3]];
			auto p1 = positions[indices[i * 3 + 1]];
			auto p2 = positions[indices[i * 3 + 2]];
			face_normals.push_back(normalize(cross(p1 - p0, p2 - p0)));
		}

		for (int i = 0; i < vertices.size(); i++) {
			auto v = vertices[i];
			auto h = v.outgoing_halfedge;
			if (h == nullptr) {
				// not in triangle
				normals->push_back(Normal(0, 1, 0));
				continue;
			}
			auto h_stop = h;
			
			Vec3 normal_sum(0, 0, 0);
			int count = 0;
			do {
				normal_sum += face_normals[h->face->id];
				count++;
				if (h->opposite_half_edge == nullptr)
					break;
				h = h->opposite_half_edge->next_half_edge;
			} while (h != h_stop);

			normals->push_back(Normal(normalize(normal_sum / count)));
		}
	}
}