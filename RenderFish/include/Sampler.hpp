#pragma once
#include "Math.hpp"
#include "Ray.hpp"
#include "Spectrum.hpp"
#include "Intersection.hpp"

struct CameraSample {
	float image_x, image_y;
	float lens_u, lens_v;
	float time;
};

struct Sample : public CameraSample {
private:

public:

};

class Sampler
{
public:
	const int x_pixel_start, x_pixel_end;
	const int y_pixel_start, y_pixel_end;
	const int samples_per_pixel;
	const float shutter_open, shutter_close;

public:
	Sampler(int x_start, int x_end, int y_start, int y_end, int samples_per_pixel,
		float shutter_open, float shutter_close)
		: x_pixel_start(x_start), x_pixel_end(x_end), y_pixel_start(y_start), y_pixel_end(y_end),
		samples_per_pixel(samples_per_pixel), shutter_open(shutter_open), shutter_close(shutter_close) {
	}

	virtual int get_more_samples(Sample *sample, RNG &rng) = 0;

	virtual int maximum_sample_count() = 0;

	bool report_results(Sample *samples, const RayDifferential &rays,
		const Spectrum *Ls, const Intersection *isects, int count) {
		return true;
	}

	virtual Sampler * get_sub_sampler(int num, int count) = 0;
	void compute_sub_window(int num, int count, int *new_x_start,
		int *new_x_end, int * nex_y_start, int *new_y_end) const;
};

