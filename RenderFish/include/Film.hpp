#pragma once
#include "RenderFish.hpp"
#include "Renderer.hpp"
#include "Sampler.hpp"
#include "Spectrum.hpp"
#include "Filter.hpp"
#include "MemoryArena.hpp"
#include "Parallel.hpp"

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
	float Lxyz[3];// = { 0, 0, 0 };
	float weight_sum;// {0};
	float splat_xyz[3];// = { 0, 0, 0 };
	float pad;	// to 16 * 4 bytes

	Pixel() {
		memset(this, 0, sizeof(*this));
	}
};

class ImageFilm : public Film {
private:
	shared_ptr<Filter> _filter;
	float crop_window[4];
	string filename;

	int x_pixel_start, y_pixel_start, x_pixel_count, y_pixel_count;

	unique_ptr< BlockedArray<Pixel, 2> > _pixels;
	static const int FILTER_TABLE_SIZE = 16;
	float filter_table[FILTER_TABLE_SIZE * FILTER_TABLE_SIZE];

public:
	ImageFilm(int x_res, int y_res, Filter *filter, const float crop[4], const string &fn, bool open_window);

	void add_sample(const CameraSample &sample, const Spectrum &L) override;

	void splat(const CameraSample &sample, const Spectrum &L) override;

	virtual void get_sample_extent(int *x_start, int *x_end,
		int *y_start, int *y_end) const override {}

	virtual void get_pixel_extent(int *x_start, int *x_end,
		int *y_start, int *y_end) const override {}

	virtual void write_image(float spalt_scale = 1.0f) override {}
};