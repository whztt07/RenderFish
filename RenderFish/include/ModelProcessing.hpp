#pragma once

#include "RenderFish.hpp"
#include "Math.hpp"

namespace ModelProcessing
{
	struct HalfEdge;

	struct Face {
		HalfEdge*	half_edge;

		// additional
		int id;
		//Normal normal;
	};

	struct Vertex {
		HalfEdge*	outgoing_halfedge = nullptr;

		//Point position;
		//Normal normal;
	};

	struct HalfEdge {
		HalfEdge*	next_half_edge;
		HalfEdge*	opposite_half_edge	= nullptr;
		Face*		face;
		Vertex*		to_vertex;	// the vertex it points to
	};

	class Mesh {
	private:
		vector<HalfEdge> half_edges;
		vector<Vertex> vertices;
		vector<Face> faces;

		vector<Point> positions;
		vector<int> indices;

	public:
		// triangles: 3 vertex ids for each face
		Mesh(vector<Point> points, vector<int> triangles);

		void generate_normals(vector<Normal>* normals);
	};

}// ModelProcessing