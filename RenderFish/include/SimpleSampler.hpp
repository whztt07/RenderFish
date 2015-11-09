#pragma once
#include "Sampler.hpp"

class SimpleSampler : public Sampler
{
private:
	int m_current_sample_pos = 0;

public:
	SimpleSampler(int x_start, int x_end, int y_start, int y_end, int samples_per_pixel = 1)
		: Sampler(x_start, x_end, y_start, y_end, samples_per_pixel) {
	}

	// maximum number of sample values returned by get_more_samples
	virtual int maximum_sample_count() override {
		return samples_per_pixel;
	}

	virtual int get_more_samples(Sample *sample, RNG &rng) override;

	virtual Sampler * get_sub_sampler(int num, int count) override;
	virtual int round_size(int size) const override {
		return size;
	}
};