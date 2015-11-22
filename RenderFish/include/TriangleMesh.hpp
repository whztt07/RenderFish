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

	// cache
	Vec3 dpdu, dpdv;
	Normal dndu, dndv;

public:
	Triangle(const Transform *o2w, const Transform *w2o,
		bool reverse_orientation, TriangleMesh *m, int n);

	virtual BBox object_bound() const override;

	virtual BBox world_bound() const override;

	virtual bool intersect(const Ray &ray, float *t_hit, float *ray_epsilon,
		DifferentialGeometry *diff_geo) const override;

	virtual bool intersect_p(const Ray &ray) const override;

	void get_uvs(Vec2 uv[3]) const;

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
	Vec2*	uv;
	Reference<Texture<float> > m_alpha_texture;	// [optional] alpha mask

	//TriangleMesh() : Shape(), n_triangles(0), n_vertices(0), alpha_texture(nullptr) { }

	//TriangleMesh(const vector<int> vi, const vector<Point> P,
	//	const vector<Normal> normal, const vector<Vec2> uv)
	//	: TriangleMesh(&Transform::identity, &Transform::identity, false, vi.size() / 3, P.size(),
	//		&vi[0], &P[0], &normal[0], nullptr, &uv[0], nullptr) {
	//}

	TriangleMesh(const Transform *o2w, const Transform *w2o,
		bool reverse_orientation, int nt, int nv, const int* vi, const Point *P,
		const Normal *normal, const Vec3 *tangent, const Vec2 *uv,
		const Reference<Texture<float> > &alpha_texture);

	TriangleMesh(const Transform *o2w, const Transform * w2o, bool reverse_orientation,
		const string& mesh_file_path, const Reference<Texture<float> > &alpha_texture);

	virtual BBox object_bound() const override {
		BBox obj_bounds;
		for (int i = 0; i < n_vertices; i++)
			obj_bounds = combine(obj_bounds, (*world_to_object)(position[i]));
		return obj_bounds;
	}

	virtual BBox world_bound() const override {
		BBox world_bounds;
		for (int i = 0; i < n_vertices; i++)
			world_bounds = combine(world_bounds, position[i]);
		return world_bounds;
	}

	virtual bool can_intersect() const override {
		return false;
	}

	virtual void refine(vector<Reference<Shape>> &refined) const override {
		for (int i = 0; i < n_triangles; ++i)
			refined.push_back(new Triangle(object_to_world, world_to_object,
				reverse_orientation, (TriangleMesh *)this, i));
	}

	friend class ModelIO;

private:
	void _set_data(int nt, int nv, const int* vi, const Point *P,
		const Normal *normal, const Vec3 *tangent, const Vec2 *uv);
};

