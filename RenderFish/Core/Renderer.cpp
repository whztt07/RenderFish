#include "Renderer.hpp"
#include "Integrator.hpp"

//void SamplerRender::render(const Scene * scene) const
//{
//	suface_integrator->preprocess(scene, camera, this);
//	Sample *sample = new Sample(sampler, suface_integrator, nullptr, scene);
//
//	// create and launch SamplerRenderTasks for rendering images
//	//    compute number of SamplerRenderTasks to create for rendering
//	int n_pixels = camera->film->x_resolution * camera->film->y_resolution;
//	int n_tasks = std::max(32 * num_system_cores(), n_pixels / (16 * 16));
//	n_tasks = round_up_pow_2(n_tasks);
//
//	vector<Task *> render_tasks;
//	for (int i = 0; i < n_tasks; ++i) {
//		render_tasks.push_back((new SamplerRenderTask(scene, this, camera,
//			sampler, sample, n_tasks - 1 - i, n_tasks));
//
//		enqueue_tasks(render_tasks);
//		wait_for_all_tasks();
//		for (uint32_t i = 0; i < render_tasks.size(); ++i)
//			delete render_tasks[i];
//	}
//
//	// clean up after rendering and store final image
//	delete sample;
//	camera->film->write_image();
//}