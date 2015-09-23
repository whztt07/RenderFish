#pragma once

#include <vector>

#include "ViewPlane.hpp"
#include "Geometry.hpp"
#include "Color.hpp"
#include "Window.hpp"

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

	void open_window(const int hres, const int vres) const
	{
		if (screen_init(hres, vres, "Hello"))
			exit(1);
	}

	void display_pixel(const int x, const int y, const Color& pixel_color) const
	{
		Assert(!(x < 0 || x > vp.hres || y < 0 || y > vp.vres));
		if (x < 0 || x > vp.hres || y < 0 || y > vp.vres)
			return;
		int p = (y * vp.hres + x)*4;
		screen_fb[p++] = pixel_color.b;
		screen_fb[p++] = pixel_color.g;
		screen_fb[p++] = pixel_color.r;
	}

	void draw_line(int x0, int y0, int x1, int y1) const
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
				display_pixel(i, j, Color::white);
				display_pixel(x1 + x0 - i, y1 + y0 - j, Color::white);
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
				display_pixel(i, j, Color::white);
				//display_pixel(x1 + x0 - i, y1 + y0 - j, Color::white);
			}
		}

	}
};