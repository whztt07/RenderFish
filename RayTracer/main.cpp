#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <chrono>

#include "RenderFish.hpp"
#include "World.hpp"
#include "Math.hpp"
#include "Ray.hpp"
#include "Geometry.hpp"
#include "ShadeRec.hpp"
#include "Transform.hpp"

using namespace std;

class DirectionalLight
{
public:
	Vec3 direction;

	DirectionalLight(const Vec3& direction) : direction(direction) {}
};

const int width = 800;
const int height = 600;

int to_int(float f)
{
	return int(clamp(f, 0, 1) * 255);
}

int main()
{
	log_system_init();
	Assert(1 == 1);
	//Assert(1 == 2);

// 	Transform t;
// 	//Assert(t.has_scale());
// 	t = Transform::scale(2, 2, 2);
// 	Assert(t.has_scale());
// 	Vec3 v(1, 2, 3);
// 	Point p(1, 2, 3);
// 	v = Transform::translate(1, 2, 3)(v);
// 	p = Transform::translate(1, 2, 3)(p);
// 	Assert(v == Vec3(1, 2, 3));
// 	Assert(p == Vec3(2, 4, 6));
// 	
// 	t = Transform::rotate(180, Vec3(1, 1, 0));
// 	v = t(Vec3(0, 1, 0));
// 	Assert(v == Vec3(1, 0, 0));
// 
// 	BBox b1(Point(1, 2, 3), Point(3, 4, 5));
// 	auto b2 = t(b1);

	float t0 = 0, t1 = 0;
	quadratic(1, 1, -6, &t0, &t1);
	Assert(t0 == -3.0f);
	Assert(t1 == 2.0f);

	info("hello %d\n", 1);
	warning("%s\n", "warning");
	error("%f\n", M_PI);

	World w;
	w.build(width, height);
	w.render_scene();

	return 0;
}

// int main()
// {
// 	if (screen_init(width, height, "Hello"))
// 		return -1;
// 
// 	Sphere sphere(Vec3(0, 0, 0), 2);
// 	Camera camera(Vec3(0, 0, -7), Vec3(0, 0, 1));
// 	DirectionalLight light0(Vec3(-1, -1, 1));
// 
// 	//FILE * f = fopen_s("image.ppm", "w");
// 	//fprintf(f, "P3\n%d %d\n%d\n", width, height, 255);
// 	//fstream fs("image.ppm", std::fstream::out);
// 	//fs << "P3\n" << width << " " << height << "\n255\n";
// 
// 	float max_depth = 12;
// 	
// 	float ka = 0.5f, kd = 0.5f, ks = 0.8f, shininess = 32;
// 
// 	//#pragma omp parallel for
// 	for (int y = 0; y < height; y++)
// 	{
// 		float sy = 1 - y * 1.0f / height;
// 		//#pragma omp parallel for
// 		for (int x = 0; x < width; x++)
// 		{
// 			//printf("%d %d ", x, y);
// 			float sx = x * 1.0f / width;
// 			auto ray = camera.ray_to(sx, sy);
// 			
// 			Vec3 c(0, 0, 0);
// 			float t;
// 			ShadeRec hit_info;
// 			if (sphere.hit(ray, t, hit_info))
// 			{
// 				//printf("hit %f\n", hit_info.distance);
// 				//c.x = c.y = c.z = clamp(hit_info.distance / max_depth);
// 				//c = hit_info.normal;
// 				auto N = normalize(hit_info.normal);
// 				auto L = normalize(-light0.direction);
// 				auto V = normalize(-ray.direction);
// 				auto R = reflect_normalized(-L, N);
// 				auto H = normalize(V + L);
// 				float VdotR = clamp(dot(H, R));
// 				float NdotL = clamp(dot(N, L));
// 				c = Vec3(kd * NdotL) + Vec3(ks * powf(VdotR, shininess));
// 				//c = Vec3(ks * powf(VdotR, shininess));
// 			}
// 			else {
// 				//printf("not hit\n");
// 			}
// 			//fprintf(f, "%d %d %d ", to_int(c.x), to_int(c.y), to_int(c.z));
// 			//fs << to_int(c.x) << " " << to_int(c.y) << " " << to_int(c.z) << " ";
// 
// 			int p = (y * width + x)*4;
// 			screen_fb[p++] = to_int(c.z);
// 			screen_fb[p++] = to_int(c.y);
// 			screen_fb[p++] = to_int(c.x);
// 		}
// 	}
// 
// 	while (screen_exit == 0 && screen_keys[VK_ESCAPE] == 0)
// 	{
// 		screen_dispatch();
// 		screen_update();
// 	}
// }