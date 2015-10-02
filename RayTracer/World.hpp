#pragma once

#include <vector>

#include "ViewPlane.hpp"
#include "Color.hpp"
#include "Window.hpp"
#include "DifferentialGeometry.hpp"

class Tracer;

class World
{
public:
	Window window;
	ViewPlane vp;
	Color	background_color;
	Tracer*	tracer;
	std::vector<Shape*> shapes;

	World() {}
	~World() {}

	void build(int width, int height);

	void render_scene(void);

	void add_shape(Shape* shape)
	{
		shapes.push_back(shape);
	}

	Color intersect(const Ray& ray) const;

	void open_window(const int hres, const int vres) 
	{
		if (window.screen_init(hres, vres, "Hello"))
			exit(1);
	}

	void set_pixel(const int x, const int y, const Color& pixel_color)
	{
		Assert(!(x < 0 || x > vp.hres || y < 0 || y > vp.vres));
// 		if (x < 0 || x > vp.hres || y < 0 || y > vp.vres)
// 			return;
		//int p = (y * vp.hres + x)*4;
		//screen_fb[p++] = pixel_color.b;
		//screen_fb[p++] = pixel_color.g;
		//screen_fb[p++] = pixel_color.r;
		window.set_pixel(x, y, pixel_color.r, pixel_color.g, pixel_color.b);
	}

	void draw_line(int x0, int y0, int x1, int y1);
};