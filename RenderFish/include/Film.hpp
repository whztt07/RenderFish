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

	virtual void update_display(int x0, int y0, int x1, int y1, float spalt_scale = 1.f) {}

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
	shared_ptr<Filter> filter;
	float crop_window[4];
	string filename;

	int x_pixel_start, y_pixel_start, x_pixel_count, y_pixel_count;

	// TODO -> blocked_array
	vector<RGBPixel> pixels;

public:
	ImageFilm(int x_res, int y_res, Filter *filter, const float crop[4], const string &fn, bool open_window)
		: Film(x_res, y_res) {

	}

	ImageFilm(int x_res, int y_res)
		//: Film(x_res, y_res), filter(nullptr), crop_window{0, 0, 1, 1}, filename("") {
		: Film(x_res, y_res), filter(nullptr), filename("") {
		x_pixel_start = 0;
		y_pixel_count = x_resolution;
		y_pixel_start = 0;
		y_pixel_count = y_resolution;
	}

	void add_sample(const CameraSample &sample, const Spectrum &L) override {
		// get sampl's raster extent
		float image_x = sample.image_x - 0.5f;
		float image_y = sample.image_y - 0.5f;
		int x0 = ceil2int(image_x);
		int x1 = floor2int(image_x);
		int y0 = ceil2int(image_y);
		int y1 = float2int(image_y);
		x0 = std::max(x0, x_pixel_start);
		x1 = std::min(x1, x_pixel_start + x_pixel_count - 1);
		y0 = std::max(y0, y_pixel_start);
		y1 = std::min(y1, y_pixel_start + y_pixel_count - 1);
		if (x1 - x0 < 0 && y1 - y0 < 0) {
			return;
		}
	}

	void splat(const CameraSample &sample, const Spectrum &L) override {
		float xyz[3];
		L.to_XYZ(xyz);
		int x = floor2int(sample.image_x), y = floor2int(sample.image_y);
		if (x < x_pixel_start || x >= x_pixel_start + x_pixel_count ||
			y < y_pixel_start || )
	}
};