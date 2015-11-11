#include "DifferentialGeometry.hpp"

void DifferentialGeometry::compute_differentials(const RayDifferential & ray) const {
	if (ray.has_differentials) {
		// estimate screen space change in p and (u, v)
		//   compute auxiliary intersection points with plane
		float d = dot(normal, p);
		float tx = (d - dot(normal, ray.rx_origin)) / dot(normal, ray.rx_direction);
		Point px = ray.rx_origin + tx * ray.rx_direction;
		float ty = (d - dot(normal, ray.ry_origin)) / dot(normal, ray.ry_direction);
		Point py = ray.ry_origin + tx * ray.ry_direction;

		dpdx = px - p;
		dpdy = py - p;

		// compute uv offset at auxiliary points
		//   initialize A, Bx, and By matrices for offset computation
		float a00, a01, a10, a11;
		float Bx0, Bx1;
		float By0, By1;
		int axes0, axes1;
		if (fabsf(normal.x) > fabsf(normal.y) && fabsf(normal.x) > fabsf(normal.z)) {
			axes0 = 1; axes1 = 2;
		}
		else if (fabsf(normal.y) > fabsf(normal.z)) {
			axes0 = 0, axes1 = 2;
		}
		else {
			axes0 = 0; axes1 = 1;
		}
		a00 = dpdu[axes0];
		a01 = dpdv[axes0];
		a10 = dpdu[axes1];
		a11 = dpdv[axes1];
		Bx0 = px[axes0] - p[axes0];
		Bx1 = px[axes1] - p[axes1];
		By0 = py[axes0] - p[axes0];
		By1 = py[axes1] - p[axes1];

		if (!solve_linear_system_2x2(a00, a01, a10, a11, Bx0, Bx1, &dudx, &dvdx)) {
			dudx = 0; dvdx = 0;
		}
		if (!solve_linear_system_2x2(a00, a01, a10, a11, By0, By1, &dudy, &dvdy)) {
			dudy = 0; dvdy = 0;
		}

	}
	else {
		dudx = dvdx = 0.f;
		dudy = dvdy = 0.f;
		dpdx = dpdy = Vec3(0, 0, 0);
	}
}
