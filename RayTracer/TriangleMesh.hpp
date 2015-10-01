#pragma once
#include "RenderFish.hpp"
#include "Math.hpp"
#include "Transform.hpp"
#include "Shape.hpp"
#include "Texture.hpp"
#include "ReferenceCounted.hpp"

class TriangleMesh;

class Triangle : public Shape
{
private:
	Reference<TriangleMesh> mesh;
	int *v;
public:
	Triangle(const Transform *o2w, const Transform *w2o,
		bool reverse_orientation, TriangleMesh *m, int n);

	virtual BBox object_bound() const override;

	virtual BBox world_bound() const override;

	virtual bool interset(const Ray &ray, float *t_hit, float *ray_epsilon,
		DifferentialGeometry *diff_geo) const override;

	virtual bool interset_p(const Ray &ray) const override;

	void get_uvs(float uv[3][2]) const;

	virtual float area() const override;

	virtual void get_shading_geometry(const Transform &obj2world, const DifferentialGeometry &dg,
		DifferentialGeometry *dg_shading) const override;
};

class TriangleMesh : public Shape
{
public:

	int		n_triangles;	// number of triangles
	int		n_vertices; // number of vertices
	int*	vertex_index; // pointer to an array of vertex indices 
	Point*	position;
	Normal* normal;
	Vec3*	tangent;	// tangent
	float*	uv;
	Reference<Texture<float> > alpha_texture;	// [optional] alpha mask

	TriangleMesh() : Shape(), n_triangles(0), n_vertices(0), alpha_texture(nullptr) {

	}

	TriangleMesh(const Transform *o2w, const Transform *w2o,
		bool reverse_orientation, int nt, int nv, const int vi, const Point *P,
		const Normal *normal, const Vec3 *tangent, const float *uv,
		const Reference<Texture<float> > &alpha_texture)
		: Shape(o2w, w2o, reverse_orientation), alpha_texture(alpha_texture),
		n_triangles(nt), n_vertices(nv)
	{
		vertex_index = new int[3 * n_triangles];
		memcpy(vertex_index, (void *)vi, 3 * n_triangles * sizeof(int));
		if (normal != nullptr) {
			this->normal = new Normal[3 * n_triangles];
			memcpy(this->normal, normal, 3 * n_triangles * sizeof(Normal));
		}
		if (tangent != nullptr) {
			this->tangent = new Vec3[3 * n_triangles];
			memcpy(this->tangent, tangent, 3 * n_triangles * sizeof(Vec3));
		}
		if (uv != nullptr) {
			this->uv = new float[2 * 3 * n_triangles];
			memcpy(this->uv, uv, 2 * 3 * n_triangles * sizeof(float));
		}
		for (int i = 0; i < n_vertices; ++i) {
			position[i] = (*object_to_world)(P[i]);
		}
	}

	virtual BBox object_bound() const override {
		BBox obj_bounds;
		for (int i = 0; i < n_triangles; i++)
			obj_bounds = combine(obj_bounds, (*world_to_object)(position[i]));
		return obj_bounds;
	}

	BBox world_bound() const {
		BBox world_bounds;
		for (int i = 0; i < n_vertices; i++)
			world_bounds = combine(world_bounds, position[i]);
		return world_bounds;
	}

	virtual bool can_intersect() const {
		return false;
	}

	void refine(vector<Reference<Shape>> &refined) const {
		for (int i = 0; i < n_triangles; ++i)
			refined.push_back(new Triangle(object_to_world, world_to_object, 
			reverse_orientation, (TriangleMesh *)this, i));
	}
};

