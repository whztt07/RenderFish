#include "Box.hpp"

//const Normal Box::face_normals[6] = { Normal(1, 0, 0), Normal(-1, 0, 0),
//Normal(0, 1, 0), Normal(0, -1, 0), Normal(0, 0, 1), Normal(0, 0, -1)
//};

bool Quad::intersect(const Ray & ray, float * t_hit, float * ray_epsilon, DifferentialGeometry * diff_geo) const {
	Ray r;
	(*world_to_object)(ray, &r);
	if (zero(r.d.z))
		return false;
	float t = -r.o.z / r.d.z;
	auto p = r(t);
	if (t > r.maxt || t < r.mint)
		return false;
	if (p.x < -width / 2.f || p.x > width / 2.f || p.y < -height / 2.f || p.y > height / 2.f)
		return false;

	*t_hit = t;
	*ray_epsilon = 5e-4f * *t_hit;
	diff_geo->shape = this;
	diff_geo->p = (*object_to_world)(r(*t_hit));
	diff_geo->normal = (*object_to_world)(Normal(0, 0, 1));
	return true;
}

void Box::refine(vector<Reference<Shape>>& refined) const {
	//static const Normal fns[6] = { Normal(1, 0, 0), Normal(-1, 0, 0),
	//	Normal(0, 1, 0), Normal(0, -1, 0), Normal(0, 0, 1), Normal(0, 0, -1)
	//};

	// TODO
	static const Transform ts[6] = {
		translate(width / 2.f, 0, 0) * rotate_y(-90),
		translate(-width / 2.f, 0, 0) * rotate_y(90),
		translate(0, height / 2.f, 0) * rotate_x(90) ,
		translate(0, -height / 2.f, 0) * rotate_x(-90),
		translate(0, 0, depth / 2.f),
		translate(0, 0, -depth / 2.f) * rotate_y(180)};
	float w[3] = { depth, width, width };
	float h[3] = { height, depth, height };
	for (int i = 0; i < 6; i++) {
		refined.push_back(new Quad(ts+i, new Transform(inverse(ts[i])), reverse_orientation, w[i/2], h[i/2]));
	}
}
