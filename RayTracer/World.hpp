#pragma once

#include <vector>

#include "ViewPlane.hpp"
#include "Geometry.hpp"
#include "Color.hpp"
#include "Window.hpp"
#include "DifferentialGeometry.hpp"

class Tracer;

class World
{
public:

	ViewPlane vp;
	Color	background_color;
	Tracer*	tracer;
	std::vector<Geometry*>	objects;
	std::vector<Shape*> shapes;

	World() {}
	~World() {}

	void build(void);

	void render_scene(void) const;

	void add_object(Geometry* object)
	{
		objects.push_back(object);
	}

	void add_shape(Shape* shape)
	{
		shapes.push_back(shape);
	}

	ShadeRec hit(const Ray& ray) const
	{
		ShadeRec sr;
		float t;
		float tmin = INFINITY;

		for (unsigned int j = 0; j < objects.size(); j++)
		{
			if (objects[j]->hit(ray, t, sr) && (t < tmin)) {
				sr.hit_an_object = true;
				tmin = t;
				sr.color = objects[j]->color;
			}
		}
		return sr;
	}

	Color intersect(const Ray& ray) const {
 		DifferentialGeometry dg;
 		float t;
 		float ray_epsilon;

		for (unsigned int j = 0; j < shapes.size(); j++) {
			auto s = shapes[j];
			if (s->interset(ray, &t, &ray_epsilon, &dg)) {
			//if (s->interset_p(ray)) {
				return Color(dg.normal);
				//return Color::white;
			}
		}
		return Color::black;
	}

	void open_window(const int hres, const int vres) const
	{
		if (screen_init(hres, vres, "Hello"))
			exit(1);
	}

	void set_pixel(const int x, const int y, const Color& pixel_color) const
	{
		Assert(!(x < 0 || x > vp.hres || y < 0 || y > vp.vres));
// 		if (x < 0 || x > vp.hres || y < 0 || y > vp.vres)
// 			return;
		int p = (y * vp.hres + x)*4;
		screen_fb[p++] = pixel_color.b;
		screen_fb[p++] = pixel_color.g;
		screen_fb[p++] = pixel_color.r;
	}

	void draw_line(int x0, int y0, int x1, int y1) const;
};