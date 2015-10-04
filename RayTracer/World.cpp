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
#if 1
	*t1 = Transform::translate(-2, 0, 0);
	//*t1 *= Transform::scale(2, 2, 2);
#else
	*t1 = Transform::scale(2, 2, 2);
	*t1 *= Transform::translate(-2, 0, 0);
#endif
	*t2 = inverse(*t1);
	auto sphere = new Sphere(t1, t2, false, 1.0f);
	auto gp = new GeometryPrimitive(sphere, &material, nullptr);
	//primitives.push_back(gp);

	// test
	auto b = sphere->world_bound();
	info("%f, %f, %f,  %f, %f, %f\n", b.pmin.x, b.pmin.y, b.pmin.z, b.pmax.x, b.pmax.y, b.pmax.z);

	Transform *t3 = new Transform(), *t4 = new Transform();
	*t3 = Transform::translate(2.5f, 0, 0);
	*t4 = inverse(*t3);
	auto sphere2 = new Sphere(t3, t4, false, 1.0f, -0.6f, 0.8f, 360);
	auto gp2 = new GeometryPrimitive(sphere2, &material, nullptr);
	primitives.push_back(gp2);

	auto mesh = ModelIO::load("teapot.obj");
	auto gp3 = new GeometryPrimitive(mesh, &material, nullptr);
	primitives.push_back(gp3);

	kdTree = new KdTree(primitives, 80, 1, 0.5f, 1, -1);
}

Color World::intersect(const Ray& ray) const
{
	Color ret_color = background_color;
	Intersection isec;

	if (kdTree->intersect(ray, &isec)) {
		ret_color = Color(isec.dg.normal);
	}
	return ret_color;
}

void World::render_scene(void)
{
	Color	pixel_color;
	float	zw	= 100.0f;

	static Camera camera(Point(0, 0, -7), Vec3(0, 0, 1), float(vp.hres) / vp.vres);

	open_window(vp.hres, vp.vres);

	info("Strat rendering scene.\n");
	float percentage = 0;

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
		if (r >= (vp.vres-1) * (percentage+0.1f)) {
			percentage += 0.1f;
			progress(percentage);
		}
	}

	info("Rendering finished!\n");
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
