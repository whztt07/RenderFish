#include "Sampler.hpp"
#include "Ray.hpp"
#include "Spectrum.hpp"
#include "Intersection.hpp"
#include "Integrator.hpp"
#include <numeric>

Sample::Sample(Sampler *sampler, SurfaceIntegrator *surf, VolumeIntegrator *vol, const Scene *scene)
{
	if (surf) surf->request_samples(sampler, this, scene);
	if (vol) vol->request_samples(sampler, this, scene);
	allocate_sample_memory();
}

Sample::~Sample()
{
	if (oneD != nullptr) {
		free_aligned(oneD[0]);
		free_aligned(oneD);
	}
}

void Sample::allocate_sample_memory()
{
	uint32_t n_parts = n1D.size() + n2D.size();
	if (n_parts == 0) {
		oneD = twoD = nullptr;
		return;
	}
	oneD = alloc_aligned<float *>(n_parts);

	int total_samples = 0;
	std::accumulate(n1D.begin(), n1D.end(), total_samples);
	std::accumulate(n2D.begin(), n2D.end(), total_samples);
	float *mem = alloc_aligned<float>(total_samples);
	for (uint32_t i = 0; i < n1D.size(); ++i) {
		oneD[i] = mem;
		mem += n1D[i];
	}
	for (uint32_t i = 0; i < n2D.size(); ++i) {
		twoD[i] = mem;
		mem += 2 * n2D[i];
	}
}

Sample * Sample::duplicate(int count) const
{
	Sample * samples = new Sample[count];
	for (int i = 0; i < count; ++i) {
		samples[i].oneD = oneD;
		samples[i].twoD = twoD;
		samples[i].allocate_sample_memory();
	}
	return samples;
}

inline void Sampler::compute_sub_window(int num, int count, int * new_x_start, int * new_x_end, int * nex_y_start, int * new_y_end) const {

	// determine how many tiles to use in each dimension, nx ans ny
	int dx = x_pixel_end - x_pixel_start, dy = y_pixel_end - y_pixel_start;
	int nx = count, ny = 1;
	// make tiles to be roughly square
	while ((nx & 0x1) == 0 && 2 * dx * ny < dy * nx) {
		nx >>= 1;
		ny <<= 1;
	}

	// compute x and y pixel smaple range for sub-window
	int xo = num % nx, yo = num / nx;
	float tx0 = float(xo) / nx, tx1 = float(xo + 1) / nx;
	float ty0 = float(yo) / ny, ty1 = float(yo + 1) / ny;
	*new_x_start = floor2int(lerp(tx0, float(x_pixel_start), float(x_pixel_end)));
	*new_x_end   = floor2int(lerp(tx1, float(x_pixel_start), float(x_pixel_end)));
	*nex_y_start = floor2int(lerp(ty0, float(y_pixel_start), float(y_pixel_end)));
	*new_y_end   = floor2int(lerp(ty1, float(y_pixel_start), float(y_pixel_end)));
}
