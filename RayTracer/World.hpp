#pragma once

#include <vector>

#include "ViewPlane.hpp"
#include "Color.hpp"
#include "Window.hpp"
#include "DifferentialGeometry.hpp"
#include "KdTree.hpp"

class Tracer;

class World
{
public:
	//Window window;
	ViewPlane vp;
	Color	background_color;
	Tracer*	tracer;
	//vector<Shape*> shapes;
	vector<Reference<Primitive>> primitives;
	KdTree* kdTree;

	Material material;

	vector<unsigned char> color_buffer;

	World() {}
	~World() {}

	void build(int width, int height);

	void render_scene(void);

	//void add_shape(Shape* shape)
	//{
	//	shapes.push_back(shape);
	//}

	Color intersect(const Ray& ray) const;

	//void open_window(const int hres, const int vres) 
	//{
	//	if (window.screen_init(hres, vres, "Hello"))
	//		exit(1);
	//}

	void set_pixel(const int x, const int y, const Color& pixel_color)
	{
		Assert(!(x < 0 || x > vp.hres || y < 0 || y > vp.vres));
// 		if (x < 0 || x > vp.hres || y < 0 || y > vp.vres)
// 			return;
		//window.set_pixel(x, y, pixel_color.r, pixel_color.g, pixel_color.b);
		int p = (y * vp.hres + x) * 4;
		color_buffer[p++] = pixel_color.b;
		color_buffer[p++] = pixel_color.g;
		color_buffer[p++] = pixel_color.r;
	}

	void draw_line(int x0, int y0, int x1, int y1);
};