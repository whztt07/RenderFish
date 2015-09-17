#include "World.h"
#include "Tracer.h"

void World::build(void)
{
	vp.set_hres(512);
	vp.set_vres(512);
	vp.set_pixel_size(1.0f);
	vp.set_gamma(1.0f);

	background_color = Color::black;
	tracer = new Tracer(this);

	auto s1 = new Sphere(Vec3(0, -25, 0), 80);
	s1->set_color(Color::red);
	auto s2 = new Sphere(Vec3(0, 30, 0), 60);
	s2->set_color(Color::yellow);
	auto p1 = new Plane(Vec3(0, 0, 0), Vec3(0, 1, 1));
	p1->set_color(Vec3(0, 0.3, 0));
	add_object(s1);
	add_object(s2);
	add_object(p1);
}

void World::render_scene(void) const
{
	Color	pixel_color;
	Ray		ray;
	float	zw	= 100.0f;
	float	x, y;
	//int n = (int)sqrt((float)vp.num_samples);

	open_window(vp.hres, vp.hres);
	ray.direction = Vec3(0, 0, -1);

	for (int r = 0; r < vp.vres; r++)
	{
		for (int c = 0; c < vp.hres; c++)
		{
			x = vp.pixel_size * (c - 0.5f * (vp.hres - 1.0));
			y = vp.pixel_size * (r - 0.5f * (vp.vres - 1.0));
			ray.origin = Vec3(x, y, zw);
			pixel_color = tracer->trace_ray(ray);
			display_pixel(r, c, pixel_color);
		}
	}

	while (screen_exit == 0 && screen_keys[VK_ESCAPE] == 0)
	{
		screen_dispatch();
		screen_update();
	}
}