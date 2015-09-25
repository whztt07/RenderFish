#pragma once
#include "RenderFish.hpp"
#include "Math.hpp"
#include "Transform.hpp"
#include "Shape.hpp"
#include "Texture.hpp"

class TriangleMesh : public Shape
{
public:

	int ntris;	// number of triangles
	int nverts; // number of vertices
	int* vertex_index; // pointer to an array of vertex indices 
	Point* p;
	Normal* n;
	Vec3 s;	// tangent
	float* uv;
	Reference<Texture<float> > alpha_texture;	// [optional] alpha mask


	TriangleMesh(const Transform *o2w, const Transform *w2o,
		bool ro, int nt, int nv, const int vi, const Point *P,
		const Normal *N, const Vec3 *S, const float *uv,
		const Reference<Texture<float> > &atex)
		: Shape(o2w, w2o, ro), alpha_texture(atex),
		ntris(nt), nverts(nv)
	{
		vertex_index = new int[3 * ntris];
		memcpy(vertex_index, (void *)vi, 3 * ntris * sizeof(int));
	}
	~TriangleMesh() {};
};

