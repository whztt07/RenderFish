#include "SimpleSampler.hpp"

int SimpleSampler::get_more_samples(Sample *sample, RNG &rng)
{
	const int w = x_pixel_end - x_pixel_start;
	const int h = y_pixel_end - y_pixel_start;
	int total_sample_count = w * h;
	if (m_current_sample_pos >= total_sample_count) {
		return 0;
	}
	sample->image_x = float(m_current_sample_pos % w) + x_pixel_start;
	sample->image_y = float(m_current_sample_pos / w) + y_pixel_start;
	//for (uint32_t i = 0; i < sample->n1D.size(); ++i)
	//	for (uint32_t j = 0; j < sample->n1D[i]; ++j)
	//		sample->oneD[i][j] = rng.random_float();
	//for (uint32_t i = 0; i < sample->n2D.size(); ++i)
	//	for (uint32_t j = 0; j < 2 * sample->n2D[i]; ++j)
	//		sample->twoD[i][j] = rng.random_float();
	++m_current_sample_pos;
	return 1;
}

Sampler * SimpleSampler::get_sub_sampler(int num, int count)
{
	int x0, x1, y0, y1;
	compute_sub_window(num, count, &x0, &x1, &y0, &y1);
	if (x0 == x1 || y0 == y1) return nullptr;
	return new SimpleSampler(x0, x1, y0, y1, samples_per_pixel);
}