#pragma once
#include "Sampler.hpp"

void StratifiedSample2D(float* samp, int nx, int ny, RNG &rng, bool jitter) {
	float dx = 1.f / nx, dy = 1.f / ny;
	for (int y = 0; y < ny; ++y)
		for (int x = 0; x < nx; ++x) {
			float jx = jitter ? rng.random_float() : 0.5f;
			float jy = jitter ? rng.random_float() : 0.5f;
			*samp++ = (x + jx) * dx;
			*samp++ = (y + jy) * dy;
		}
}

class StratifiedSampler : public Sampler 
{
private:
	int m_x_pixel_samples;
	int m_y_pixel_samples;
	int m_x_pos, m_y_pos;
	float *m_sample_buf;
	bool m_jitter_samples; // whether the samples should be jittered

public:
	StratifiedSampler(int x_start, int x_end, int y_start, int y_end, 
		int samples_per_pixel_x, int samples_per_pixel_y, bool jitter)
		: Sampler(x_start, x_end, y_start, y_end, samples_per_pixel_x * samples_per_pixel_y), 
		m_x_pixel_samples(samples_per_pixel_x), m_y_pixel_samples(samples_per_pixel_y),
		m_x_pos(x_pixel_start), m_y_pos(y_pixel_start), m_jitter_samples(jitter){

		m_sample_buf = new float[2 * m_x_pixel_samples * m_y_pixel_samples];
	}

	~StratifiedSampler() {
		delete[] m_sample_buf;
	}

	virtual int maximum_sample_count() override {
		return samples_per_pixel;	//m_x_pixel_samples * m_y_pixel_samples
	}

	virtual int round_size(int size) const override {
		return size;
	}

	virtual Sampler* get_sub_sampler(int num, int count) override {
		int x0, x1, y0, y1;
		compute_sub_window(num, count, &x0, &x1, &y0, &y1);
		if (x0 == x1 || y0 == y1) return nullptr;
		return new StratifiedSampler(x0, x1, y0, y1, m_x_pixel_samples, m_y_pixel_samples, m_jitter_samples);
	}

	virtual int get_more_samples(Sample *sample, RNG &rng) override {
		if (m_y_pos == y_pixel_end) return 0;
		// generate stratified camera samples for (m_x_pos, m_y_pos);
		float *image_samples = m_sample_buf;
		StratifiedSample2D(image_samples, m_x_pixel_samples, m_y_pixel_samples, rng, m_jitter_samples);
		for (int i = 0; i < samples_per_pixel; ++i) {
			sample[i].image_x = m_x_pos + image_samples[2 * i];
			sample[i].image_y = m_y_pos + image_samples[2 * i + 1];
		}
		// advance to next pixel for stratified sampling
		if (++m_x_pos == x_pixel_end) {
			m_x_pos = x_pixel_start;
			++m_y_pos;
		}
		return samples_per_pixel;
	}
};