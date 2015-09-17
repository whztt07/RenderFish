#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstdlib>

#ifdef _MSC_VER
//#include "rtwindow.h"
#endif

#include "World.h"
#include "MathHelper.h"
#include "Ray.h"
#include "Geometry.h"
#include "ShadeRec.h"

// Left hand
// screen coord: (0, 0 ) at bottom, left, (1, 1) at top right 
using namespace std;

class DirectionalLight
{
public:
	Vec3 direction;

	DirectionalLight(const Vec3& direction) : direction(direction) {}
};

class Transform {
public:
	Vec3 pos;
	//Quaternion rot;
	Vec3 scale;
};


class Camera
{
public:
	Vec3 eye;
	Vec3 front;
	Vec3 right;
	Vec3 up = Vec3(0, 1, 0);

	float z_far		= 0.1f;
	float z_near	= 100.0f;
	float fov		= 45.0f;
	float fovScale;

	Camera(const Vec3& eye, const Vec3& front) 
		: eye(eye), front(front)
	{
		fovScale = tanf(fov * 0.5f * PI / 180.0f) * 2;
		right = cross(up, front);
		up = cross(front, right);
	}

	Ray ray_to(float x, float y)
	{
		auto r = right * ((x - 0.5f) * fovScale);
		auto u = up * ((y - 0.5f) * fovScale);
		return Ray(eye, (front + r + u).normalize());
	}
};

const int width = 512;
const int height = 512;

int to_int(float f)
{
	return int(clamp(f) * 255);
}

int main()
{
	World w;
	w.build();
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