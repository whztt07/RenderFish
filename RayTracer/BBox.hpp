#ifndef BBOX_H
#define BBOX_H
#include "Math.hpp"

class Ray;

class BBox
{
public:
	Point pmin, pmax;

	enum Axis {
		x_axis = 0,
		y_axis = 1,
		z_axis = 2
	};

	BBox() 
		: pmin(INFINITY, INFINITY, INFINITY), pmax(-INFINITY, -INFINITY, -INFINITY) {};
	BBox(const Point &p) 
		: pmin(p), pmax(p) {}
	BBox(const Point &p1, const Point &p2) {
		pmin = Point(fmin(p1.x, p2.x), fmin(p1.y, p2.y), fmin(p1.z, p2.z));
		pmax = Point(fmax(p1.x, p2.x), fmax(p1.y, p2.y), fmax(p1.z, p2.z));
	}

	bool intersect_p(const Ray &ray, float *hit_t0, float *hit_t1) const;

	friend BBox combine(const BBox &b, const Point &p)
	{
		BBox ret;
		ret.pmin.x = fmin(b.pmin.x, p.x);
		ret.pmin.y = fmin(b.pmin.y, p.y);
		ret.pmin.z = fmin(b.pmin.z, p.z);
		ret.pmax.x = fmax(b.pmax.x, p.x);
		ret.pmax.y = fmax(b.pmax.y, p.y);
		ret.pmax.z = fmax(b.pmax.z, p.z);
		return ret;
	}

	friend BBox combine(const BBox &b1, const BBox &b2)
	{
		BBox ret;
		ret.pmin.x = fmin(b1.pmin.x, b2.pmin.x);
		ret.pmin.y = fmin(b1.pmin.y, b2.pmin.y);
		ret.pmin.z = fmin(b1.pmin.z, b2.pmin.z);
		ret.pmax.x = fmax(b1.pmax.x, b2.pmax.x);
		ret.pmax.y = fmax(b1.pmax.y, b2.pmax.y);
		ret.pmax.z = fmax(b1.pmax.z, b2.pmax.z);
        return ret;
	}

	bool overlaps(const BBox &b) const {
		bool x = (pmax.x >= b.pmin.x) && (pmin.x <= b.pmax.x);
		bool y = (pmax.y >= b.pmin.y) && (pmin.y <= b.pmax.y);
		bool z = (pmax.z >= b.pmin.z) && (pmin.z <= b.pmax.z);
		return (x && y && z);
	}

	bool inside(const Point &p) const {
		return (p.x >= pmin.x && p.x <= pmax.x &&
			p.y >= pmin.y && p.y <= pmax.y &&
			p.z >= pmin.z && p.z <= pmax.z);
	}

	void expand(float delta) {
		pmin -= Vec3(delta, delta, delta);
		pmax += Vec3(delta, delta, delta);
	}

	float surface_area() const {
		auto d = pmax - pmin;
		return 2.0f * (d.x * d.y + d.x * d.z + d.y * d.z);
	}

	float volume() const {
		auto d = pmax - pmin;
		return d.x * d.y * d.z;
	}

	Axis maximum_extent() const {
		auto diag = pmax - pmin;
		if (diag.x > diag.y && diag.x > diag.z)
			return Axis::x_axis;
		else if (diag.y > diag.z)
			return Axis::y_axis;
		return Axis::z_axis;
	}

	const Point& operator[](int i) const {
		return i == 0 ? pmin : pmax;
	}
	Point& operator[](int i) {
		return i == 0 ? pmin : pmax;
	}

	Point lerp(float tx, float ty, float tz) const {
		return Point(::lerp(tx, pmin.x, pmax.x), ::lerp(ty, pmin.y, pmax.y), ::lerp(tz, pmin.z, pmax.z));
	}

	Vec3 offset(const Point& p) const {
		return Vec3((p.x - pmin.x) / (pmax.x - pmin.x),
			(p.y - pmin.y) / (pmax.y - pmin.y),
			(p.z - pmin.z) / (pmax.z - pmin.z));
	}

	void bounding_sphere(Point *c, float *rad) const {
		*c = center(pmin, pmax);
		*rad = inside(*c) ? distance(*c, pmax) : 0.0f;
	}
};

#endif // BBOX_H