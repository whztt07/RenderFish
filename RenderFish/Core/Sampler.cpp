#include "Sampler.hpp"
#include "Ray.hpp"
#include "Spectrum.hpp"
#include "Intersection.hpp"
#include "Integrator.hpp"


Sample::Sample(Sampler *sampler, SurfaceIntegrator *surf, VolumeIntegrator *vol, const Scene *scene)
{
	if (surf) surf->request_samples(sampler, this, scene);
	if (vol) vol->request_samples(sampler, this, scene);
	allocate_sample_memory();
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
