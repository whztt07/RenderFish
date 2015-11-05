#include "Film.hpp"

ImageFilm::ImageFilm(int x_res, int y_res, Filter *filter, const float crop[4], const string &fn, bool open_window) 
	: Film(x_res, y_res), _filter(filter), filename(fn)
{
	memcpy(crop_window, crop, 4 * sizeof(float));
	// compute film image extent
	x_pixel_start = ceil2int(x_resolution * crop_window[0]);
	x_pixel_count = max(1, ceil2int(x_resolution * crop_window[1]) - x_pixel_start);
	y_pixel_start = ceil2int(y_resolution * crop_window[2]);
	y_pixel_count = max(1, ceil2int(y_resolution * crop_window[3]) - y_pixel_start);

	// allocate film image storage
	_pixels = std::make_unique<BlockedArray<Pixel, 2>>(x_pixel_count, y_pixel_count);
	// precompute filter weight table
	float *ftp = filter_table;
	for (int y = 0; y < FILTER_TABLE_SIZE; ++y) {
		float fy = ((float)y + 0.5f) * filter->y_width / FILTER_TABLE_SIZE;
		for (int x = 0; x < FILTER_TABLE_SIZE; ++x) {
			float fx = ((float)x + 0.5f) * filter->x_width / FILTER_TABLE_SIZE;
			*ftp++ = filter->evaluate(fx, fy);
		}
	}

	// possibly open window for image display
}

void ImageFilm::add_sample(const CameraSample &sample, const Spectrum &L)
{
	// get sample's raster extent
	float image_x = sample.image_x - 0.5f;
	float image_y = sample.image_y - 0.5f;
	int x0 = ceil2int (image_x - _filter->x_width);
	int x1 = floor2int(image_x + _filter->x_width);
	int y0 = ceil2int (image_y - _filter->y_width);
	int y1 = float2int(image_y + _filter->y_width);
	x0 = max(x0, x_pixel_start);
	x1 = min(x1, x_pixel_start + x_pixel_count - 1);
	y0 = max(y0, y_pixel_start);
	y1 = min(y1, y_pixel_start + y_pixel_count - 1);
	if (x1 - x0 < 0 && y1 - y0 < 0) {
		return;
	}

	// loop over filter support and add sample to pixel arrays
	float xyz[3];
	L.to_XYZ(xyz);
	// precompute x and y filter table offsets
	bool sync_needed = (_filter->x_width > 0.5f || _filter->y_width > 0.5f);
	for (int y = y0; y <= y1; ++y) {
		for (int x = x0; x <= x1; ++x) {
			// evaluate filter value at (x,y) pixel
			// update pixel values with filtered sample contribution
		}
	}
}

void ImageFilm::splat(const CameraSample &sample, const Spectrum &L)
{
	float xyz[3];
	L.to_XYZ(xyz);
	int x = floor2int(sample.image_x), y = floor2int(sample.image_y);
	if (x < x_pixel_start || x >= x_pixel_start + x_pixel_count ||
		y < y_pixel_start || y >= y_pixel_start + y_pixel_count)
		return;
	auto& pixel = (*_pixels)(x - x_pixel_start, y - y_pixel_start);
	atomic_add(&pixel.splat_xyz[0], xyz[0]);
	atomic_add(&pixel.splat_xyz[1], xyz[1]);
	atomic_add(&pixel.splat_xyz[2], xyz[2]);
}