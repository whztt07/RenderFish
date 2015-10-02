#pragma once
#include "ReferenceCounted.hpp"
#include "BBox.hpp"
#include "Intersection.hpp"
#include "DifferentialGeometry.hpp"
#include "Material.hpp"
#include "AreaLight.hpp"

class Primitive : public ReferenceCounted
{
protected:
	static uint32_t next_primitive_id;
public:
	const uint32_t primitive_id;

public:
	Primitive() : primitive_id(next_primitive_id++) {}

	virtual BBox world_bound() const = 0;
	virtual bool can_intersect() const = 0;
	virtual bool intersect(const Ray &r, Intersection *isect) const = 0;
	virtual bool intersect_p(const Ray &r) const = 0;
	virtual void refine(vector<Reference<Primitive>> &refined) const {
		error("Unimplemented Primivite::refine() method called\n");
	}
	void fully_refine(vector<Reference<Primitive>> &refined) const;
	virtual const AreaLight *get_area_light() const {
		return nullptr;
	}
	//virtual BSDF *GetBSDF(const DifferentialGeometry &dg,
	//	const Transform &ObjectToWorld, MemoryArena &arena) const = 0;
	//virtual BSSRDF *GetBSSRDF(const DifferentialGeometry &dg,
	//	const Transform &ObjectToWorld, MemoryArena &arena) const = 0;
};

class GeometryPrimitive : public Primitive
{
private:
	Reference<Shape> _shape;
	Reference<Material> _material;
	AreaLight *_area_light;
public:
	GeometryPrimitive(const Reference<Shape> &shape, const Reference<Material> &material, AreaLight* area_light)
		: _shape(shape), _material(material), _area_light(area_light) {
		
	}

	virtual BBox world_bound() const override {
		return _shape->world_bound();
	}

	virtual bool can_intersect() const override {
		return _shape->can_intersect();
	}

	virtual bool intersect(const Ray &r, Intersection *isect) const override;
	virtual bool intersect_p(const Ray &r) const override {
		if (!_shape->intersect_p(r))
			return false;
		return true;
	}
	virtual void refine(vector<Reference<Primitive>> &refined) const {
		vector<Reference<Shape>> vs;
		_shape->refine(vs);
		for (auto i = vs.begin(); i != vs.end(); ++i) {
			refined.push_back(new GeometryPrimitive(*i, _material, _area_light));
		}
	}
	virtual const AreaLight *get_area_light() const {
		return _area_light;
	}
};