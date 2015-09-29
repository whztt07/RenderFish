#include "World.hpp"
#include "Tracer.hpp"
#include "Shape.hpp"
#include "Camera.hpp"

void World::build(void)
{
	vp.set_hres(512);
	vp.set_vres(512);
	vp.set_pixel_size(1.0f);
	vp.set_gamma(1.0f);

	background_color = Color::black;
	tracer = new Tracer(this);
	auto p1 = new Plane(Vec3(0, 0, 0), Vec3(0, 1, 1));
	p1->set_color(Color(Vec3(0, 0.3f, 0)));
	add_object(p1);

	Transform *t1 = new Transform(), *t2 = new Transform();
	*t1 = Transform::translate(1, 2, 0);
	*t2 = inverse(*t1);
	auto sphere = new Sphere(t1, t2, false, 2.0f);
	add_shape(sphere);
}

void World::render_scene(void) const
{
	Color	pixel_color;
	float	zw	= 100.0f;

	static Camera camera(Point(0, 0, -7), Vec3(0, 0, 1));

	open_window(vp.hres, vp.vres);

	for (int r = 0; r < vp.vres; r++)
	{
		float sy = 1 - r * 1.0f / vp.vres;
		for (int c = 0; c < vp.hres; c++)
		{
			float sx = c * 1.0f / vp.hres;
			auto ray = camera.ray_to(sx, sy);
			pixel_color = tracer->trace_ray(ray);
			set_pixel(c, r, pixel_color);
		}
	}

	//draw_line(vp.hres, 10, 0, vp.vres / 2);
	//display_pixel(20, 1, Color::white);

	while (screen_exit == 0 && screen_keys[VK_ESCAPE] == 0)
	{
		screen_dispatch();
		screen_update();
		static float elapse = 1.0f / 30 * 1000;
		Sleep(DWORD(elapse));
	}
}

void World::draw_line(int x0, int y0, int x1, int y1) const
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