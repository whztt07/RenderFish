#include "BBox.hpp"
#include "Ray.hpp"

bool BBox::intersect_p(const Ray & ray, float * hit_t0, float * hit_t1) const
{
	float t0 = ray.mint, t1 = ray.maxt;

	for (int i = 0; i < 3; i++) {
		float inv_d = 1.f / ray.d[i];
		float tt0 = (pmin[i] - ray.o[i]) * inv_d;
		float tt1 = (pmax[i] - ray.o[i]) * inv_d;
		if (tt0 > tt1)
			std::swap(tt0, tt1);
		if (t0 < tt0) t0 = tt0;
		if (t1 > tt1) t1 = tt1;
		if (t0 > t1) return false;
	}

	if (hit_t0 != nullptr) *hit_t0 = t0;
	if (hit_t1 != nullptr) *hit_t1 = t1;
	return true;
}
