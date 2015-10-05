#pragma once
#include "Sampler.hpp"
#include "Spectrum.hpp"
#include "Filter.hpp"

class Film
{
public:
	const int x_resolution, y_resolution;
public:
	Film(int x_res, int y_res)
		: x_resolution(x_res), y_resolution(y_res) {}

	virtual void add_sample(const CameraSample &sample, const Spectrum &L) = 0;

	virtual void splat(const CameraSample &sample, const Spectrum &L) = 0;

	virtual void get_sample_extent(int *x_start, int *x_end,
		int *y_start, int *y_end) const = 0;

	virtual void get_pixel_extent(int *x_start, int *x_end,
		int *y_start, int *y_end) const = 0;

	virtual void update_display(int x0, int y0, int x1, int y1, float spalt_scale) {}

	virtual void write_image(float spalt_scale = 1.0f) = 0;
};


struct Pixel {
	float Lxyz[3];
	float weight_sum;
	float splat_xyz[3];
	float pad;
};

#include "Color.hpp"
struct RGBPixel {
	Color rgb;
};

class ImageFilm : public Film {
private:
	Filter *filter;
	float crop_window[4];
	string filename;

	int x_pixel_start, y_pixel_start, x_pixel_count, y_pixel_count;

public:
	ImageFilm(int x_res, int y_res, Filter *filter, const float crop[4], const string &fn, bool open_window)
		: Film(x_res, y_res) {

	}

	ImageFilm(int x_res, int y_res)
		: Film(x_res, y_res), filter(nullptr), crop_window{0, 0, 1, 1}, filename("") {
		x_pixel_start = 0;
		y_pixel_count = x_resolution;
		y_pixel_start = 0;
		y_pixel_count = y_resolution;
	}
};