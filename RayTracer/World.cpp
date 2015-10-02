#include "World.hpp"
#include "Tracer.hpp"
#include "Shape.hpp"
#include "Sphere.hpp"
#include "Camera.hpp"
#include "TriangleMesh.hpp"
#include "ModelIO.hpp"

void World::build( int width, int height)
{
	vp.set_hres(width);
	vp.set_vres(height);
	vp.set_pixel_size(1.0f);
	vp.set_gamma(1.0f);

	background_color = Color::black;
	tracer = new Tracer(this);

	// TODO: Transform pool
	Transform *t1 = new Transform(), *t2 = new Transform();
	*t1 = Transform::translate(-2, 0, 0);
	*t2 = inverse(*t1);
	auto sphere = new Sphere(t1, t2, false, 1.0f);
	add_shape(sphere);

	Transform *t3 = new Transform(), *t4 = new Transform();
	*t3 = Transform::translate(2, 0, 0);
	*t4 = inverse(*t3);
	auto sphere2 = new Sphere(t3, t4, false, 1.0f, -0.6f, 0.8f, 360);
	add_shape(sphere2);

	TriangleMesh *mesh = new TriangleMesh();
	ModelIO::load("cube.obj", mesh);
	add_shape(mesh);
}

Color World::intersect(const Ray& ray) const
{
	DifferentialGeometry dg;
	float t;
	float ray_epsilon;
	Color ret_color = background_color;

	for (unsigned int j = 0; j < shapes.size(); j++) {
		auto s = shapes[j];
		if (s->can_intersect()) {
			if (s->intersect(ray, &t, &ray_epsilon, &dg)) {
				ray.maxt = t;
				ret_color = Color(dg.normal);
			}
		}
		else {
			//vector<Reference<Shape>> vs;
			//s->refine(vs);
			auto& vs = s->refined_shapes;
			for (auto i = vs.begin(); i != vs.end(); ++i) {
				if ((*i)->intersect(ray, &t, &ray_epsilon, &dg)) {
					ray.maxt = t;
					ret_color = Color::green;
				}
			}
		}
	}
	return ret_color;
}

void World::render_scene(void)
{
	Color	pixel_color;
	float	zw	= 100.0f;

	static Camera camera(Point(0, 0, -7), Vec3(0, 0, 1), float(vp.hres) / vp.vres);

	open_window(vp.hres, vp.vres);

	for (int r = 0; r < vp.vres; r++)
	{
		float sy = 1 - r * 1.0f / vp.vres;
		for (int c = 0; c < vp.hres; c++)
		{
			float sx = c * 1.0f / vp.vres;
			auto ray = camera.ray_to(sx, sy);
			pixel_color = tracer->trace_ray(ray);
			set_pixel(c, r, pixel_color);
		}
	}
	window.run();
}

void World::draw_line(int x0, int y0, int x1, int y1)
{
	if (x0 > x1)
	{
		int t = x1;
		x1 = x0; x0 = t; t = y1; y1 = y0; y0 = t;
	}
	// version 0: naive
	//float delta = 1.0f * (y1 - y0) / (x1 - x0);
	//float y = float(y0);
	//for (int i = x0, j = y0; i <= x1; i++)
	//{
	//	display_pixel(i, j, Color::white);
	//	j = int(y);
	//	y += delta;
	//}

	// version 1:
	//float e = 0;
	//float k = 1.0f * (y1 - y0) / (x1 - x0);
	//for (int i = x0, j = y0; i <= x1; i++)
	//{
	//	if (e > 0)
	//	{
	//		e -= 1;
	//		j++;
	//	}
	//	e += k;
	//	display_pixel(i, j, Color::white);
	//}

	// version 2: Bresenham
	// Point: float k -> int k
	//int e = 0;
	//int dx = x1 - x0;
	//int dy = y1 - y0;
	//if (y0 < y1)
	//{
	//	for (int i = x0, j = y0; i <= x1; i++)
	//	{
	//		if (e > 0)
	//		{
	//			e -= dx;
	//			j++;
	//		}
	//		e += dy;
	//		display_pixel(i, j, Color::white);
	//	}
	//}
	//else
	//{
	//	for (int i = x0, j = y0; i <= x1; i++)
	//	{
	//		if (e < 0)
	//		{
	//			e += dx;
	//			j--;
	//		}
	//		e += dy;
	//		display_pixel(i, j, Color::white);
	//	}
	//}


	// version 3: more than Bresenham 
	int e = 0;
	int dx = x1 - x0;
	int dy = y1 - y0;
	if (y0 < y1)
	{
		for (int i = x0, j = y0; i <= x1 / 2; i++)
		{
			if (e > 0)
			{
				e -= dx;
				j++;
			}
			e += dy;
			set_pixel(i, j, Color::white);
			set_pixel(x1 + x0 - i, y1 + y0 - j, Color::white);
		}
	}
	else
	{
		for (int i = x0, j = y0; i <= x1; i++)
		{
			if (e < 0)
			{
				e += dx;
				j--;
			}
			e += dy;
			set_pixel(i, j, Color::white);
			//display_pixel(x1 + x0 - i, y1 + y0 - j, Color::white);
		}
	}
}
