#pragma once

#include "RenderFish.hpp"
#include "Color.hpp"

class ImageIO
{
public:
	static void write_image(const string &name, Color *pixels)
	{

	}

	static void write_image(const string &name, float *pixels, float *alpha,
		int xres, int yres, int total_xres, int total_yres, int x_offset, int y_offset)
	{

	}

private:
	ImageIO();
};

