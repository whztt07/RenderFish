#include "RenderFish.hpp"
#include "Renderer.hpp"
#include "Integrator.hpp"
#include "Scene.hpp"
#include "Camera.hpp"
#include "Light.hpp"

void SamplerRenderer::render(const Scene * scene)
{
	m_suface_integrator->preprocess(scene, m_camera, this);
	if (m_volume_integrator != nullptr) m_volume_integrator->preprocess(scene, m_camera, this);
	Sample *sample = new Sample(m_sampler, m_suface_integrator, m_volume_integrator, scene);

	// create and launch SamplerRenderTasks for rendering images
	//    compute number of SamplerRenderTasks to create for rendering
	int n_pixels = m_camera->film->x_resolution * m_camera->film->y_resolution;
	int n_tasks = max(32 * num_system_cores(), n_pixels / (16 * 16));
	n_tasks = round_up_pow_2(n_tasks);

	vector<Task *> render_tasks;
	for (int i = 0; i < n_tasks; ++i) {
		render_tasks.push_back(new SamplerRendererTask( scene, this, m_camera,
														m_sampler, sample, n_tasks - 1 - i, n_tasks));
	}
	enqueue_tasks(render_tasks);
	wait_for_all_tasks();
	
	// clean up after rendering and store final image
	for (auto rt : render_tasks)
		delete rt;
	delete sample;
	m_camera->film->write_image();
}

Spectrum SamplerRenderer::Li(const Scene *scene, const RayDifferential &ray, const Sample *sample, RNG &rng, MemoryArena &arena, Intersection *isect /*= nullptr*/, Spectrum *T /*= nullptr*/) const
{
	// allocate local var
	Spectrum local_T;
	if (!T) T = &local_T;
	Intersection local_isect;
	Spectrum Li = 0.f;
	
	if (!isect) isect = &local_isect;

	if (scene->intersect(ray, isect))
		Li = m_suface_integrator->Li(scene, this, ray, *isect, sample, rng, arena);
	else {
		// handle ray that doesn't intersect any geometry
		for (uint32_t i = 0; i < scene->lights.size(); ++i)
			Li += scene->lights[i]->Le(ray);
	}
	// TODO
	//Spectrum Lvi = volume_integrator->Li(scene, this, ray, sample, rng, T, arena);
	Spectrum Lvi(0.f);
	return *T * Li + Lvi;
}

void SamplerRendererTask::run()
{
	// get sub-sampler
	Sampler * sampler = m_main_sampler->get_sub_sampler(m_task_number, m_task_count);
	if (sampler == nullptr)
		return;

	// local var
	MemoryArena arena;
	RNG rng(m_task_number);

	// allocate space for samples and intersections
	int max_samples = sampler->maximum_sample_count();
	Sample * samples = m_orignal_sample->duplicate(max_samples);
	RayDifferential *rays = new RayDifferential[max_samples];
	Spectrum *Ls = new Spectrum[max_samples];
	Spectrum *Ts = new Spectrum[max_samples];
	Intersection *isects = new Intersection[max_samples];

	// get samples from Sampler and update image
	int sample_count;
	while ((sample_count = sampler->get_more_samples(samples, rng)) > 0) {
		// generate camera rays and compute radiance along rays
		for (int i = 0; i < sample_count; ++i) {
			// find camera ray for sample[i]
			float ray_weight = m_camera->gererate_ray_differential(samples[i], &rays[i]);
			rays[i].scale_differentials(1.f / sqrtf((float)sampler->samples_per_pixel));
			//Ray& r = rays[i];
			// evaluate radiance along camera ray
			if (ray_weight > 0.f) {
				Ls[i] = ray_weight * m_renderer->Li(m_scene, rays[i], &samples[i], rng, arena, &isects[i], &Ts[i]);
			}
			else {
				Ls[i] = 0.f;
				Ts[i] = 1.f;
			}
			// Issue warning if unexpected radiance value returned
			if (Ls[i].has_NaNs()) {
				error("Not-a-number radiance value returned "
					"for image sample.  Setting to black.");
				Ls[i] = Spectrum(0.f);
			}
			else if (Ls[i].y() < -1e-5) {
				error("Negative luminance value, %f, returned "
					"for image sample.  Setting to black.", Ls[i].y());
				Ls[i] = Spectrum(0.f);
			}
			else if (isinf(Ls[i].y())) {
				error("Infinite luminance value returned "
					"for image sample.  Setting to black.");
				Ls[i] = Spectrum(0.f);
			}
		}
		// report sample results to Sampler, add contributions to image
		if (sampler->report_results(samples, rays, Ls, isects, sample_count)) {
			for (int i = 0; i < sample_count; ++i) {
				m_camera->film->add_sample(samples[i], Ls[i]);
			}
		}
		// free MemoryArena memory from computing image sample values
		arena.free_all();
	}

	// clean up
	m_camera->film->update_display( sampler->x_pixel_start, sampler->y_pixel_start,
									sampler->x_pixel_end + 1, sampler->y_pixel_end + 1);
	delete sampler;
	delete[] samples;
	delete[] rays;
	delete[] Ls;
	delete[] Ts;
	delete[] isects;
}