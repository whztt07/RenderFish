#pragma once
#include "RenderFish.hpp"
#include "Math.hpp"
#include "Transform.hpp"

class TriangleMesh : public Shape
{
public:

	int nt;	// number of triangles
	int nv; // number of vertices
	Point* vi; // pointer to an array of vertex indices 
	Point* P;
	vector<Normal> N;
	vector<Normal> S;	// tangent
	vector<Vec2> uv;
	//Texture atex;	// [optional] alpha mask


	TriangleMesh(const Transform *o2w, const Transform *w2o, 
		bool ro, int nt, int nv, const int vi, const Point *P, 
		const Normal *N, const Vec3 *S, const float *uv, const Reference<Texture<float>> &atex);
	~TriangleMesh();
};

