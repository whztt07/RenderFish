#pragma once

#include <vector>

#include "ViewPlane.h"
#include "Geometry.h"
#include "Color.h"

#include "rtwindow.h"

class Tracer;

class World
{
public:

	ViewPlane vp;
	Color	background_color;
	Tracer*	tracer;
	std::vector<Geometry*>	objects;

	World() {}
	~World() {}

	void build(void);

	void render_scene(void) const;

	void add_object(Geometry* object)
	{
		objects.push_back(object);
	}

	ShadeRec hit(const Ray& ray) const
	{
		ShadeRec sr;
		float t;
		float tmin = INFINITY;
		
		for (int j = 0; j < objects.size(); j++)
		{
			if (objects[j]->hit(ray, t, sr) && (t < tmin)) {
				sr.hit_an_object = true;
				tmin = t;
				sr.color = objects[j]->color;
			}
		}
		return sr;
	}

	void open_window(const int hres, const int vres) const
	{
		if (screen_init(hres, vres, "Hello"))
			exit(1);
	}

	void display_pixel(const int x, const int y, const Color& pixel_color) const
	{
		int p = (y * vp.hres + x)*4;
		screen_fb[p++] = pixel_color.b;
		screen_fb[p++] = pixel_color.g;
		screen_fb[p++] = pixel_color.r;
	}
};