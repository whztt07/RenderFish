#pragma once
#include "Sampler.hpp"
#include "MemoryArena.hpp"

class RandomSampler : public Sampler {
private:
	int x_pos;
	int y_pos;
	//int n_samples = 4;
	int sample_pos = 0;	// current sample # in a pixel(eg. each pixel may generate 5 samples, so sample_pos is in [0, 4]);
	//int pixel_pos = 0;	// position of pixel to be generate;
	//float *image_samples;

public:

	RandomSampler(int x_start, int x_end, int y_start, int y_end, int samples_per_pixel)
		: Sampler(x_start, x_end, y_start, y_end, samples_per_pixel), x_pos(x_pixel_start), y_pos(y_pixel_start) {

		//image_samples = alloc_aligned<float>(5 * n_samples);
	}

	virtual int maximum_sample_count() override {
		return 1;
	}

	virtual int round_size(int size) const override {
		return size;
	}

	virtual Sampler* get_sub_sampler(int num, int count) override {
		int x0, x1, y0, y1;
		compute_sub_window(num, count, &x0, &x1, &y0, &y1);
		if (x0 == x1 || y0 == y1) return nullptr;
		return new RandomSampler(x0, x1, y0, y1, samples_per_pixel);
	}

	virtual int get_more_samples(Sample *sample, RNG &rng) override {
		//const int w = x_pixel_end - x_pixel_start;
		//const int h = y_pixel_end - y_pixel_start;
		if (sample_pos == samples_per_pixel) { // to next pixel
			if (++x_pos == x_pixel_end) {	// -> next line
				x_pos = x_pixel_start;
				++y_pos;
			}
			if (y_pos == y_pixel_end)	// all samples in all pixels has been generate
				return 0;
			sample_pos = 0;
		}

		sample->image_x = x_pos + rng.random_float();
		sample->image_y = y_pos + rng.random_float();
		++sample_pos;
		return 1;
	}
};