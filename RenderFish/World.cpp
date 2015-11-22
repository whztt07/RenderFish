#include "World.hpp"
#include "Shape.hpp"
#include "Sphere.hpp"
#include "Camera.hpp"
#include "TriangleMesh.hpp"
#include "Box.hpp"
#include "Timer.hpp"

void World::build( int width, int height)
{
	log_group("Build world");

	ConstantTexture<Spectrum> * diffuse = new ConstantTexture<Spectrum>(Spectrum::from_rgb(1.f, 1.f, 1.f));
	MatteMaterial * material = new MatteMaterial(diffuse, nullptr, nullptr);

//	// TODO: Transform pool
//	Transform *t1 = new Transform(), *t2 = new Transform();
//#if 1
//	*t1 = translate(-2, 0, 0);
//	//*t1 *= Transform::scale(2, 2, 2);
//#else
//	*t1 = Transform::scale(2, 2, 2);
//	*t1 *= Transform::translate(-2, 0, 0);
//#endif
//	*t2 = inverse(*t1);
//	auto sphere = new Sphere(t1, t2, false, 1.0f);
//	auto gp = new GeometryPrimitive(sphere, material, nullptr);
//	primitives.push_back(gp);
//
//	// test
//	auto b = sphere->world_bound();
//	info("%f, %f, %f,  %f, %f, %f\n", b.pmin.x, b.pmin.y, b.pmin.z, b.pmax.x, b.pmax.y, b.pmax.z);
//
//	Transform *t3 = new Transform(), *t4 = new Transform();
//	*t3 = translate(1.f, 0, 0);
//	*t4 = inverse(*t3);
//	auto sphere2 = new Sphere(t3, t4, false, 1.0f, -0.6f, 0.8f, 360);
//	auto gp2 = new GeometryPrimitive(sphere2, material, nullptr);
//	primitives.push_back(gp2);
//
//	auto mesh = ModelIO::load("teapot.obj");
//	auto gp3 = new GeometryPrimitive(mesh, material, nullptr);
//	primitives.push_back(gp3);
//
//	//auto box = new Box(2, 2, 1);
//	//auto gp4 = new GeometryPrimitive(box, &material, nullptr);
//	//primitives.push_back(gp4);
//	auto mesh2 = ModelIO::load("../Models/head_optimized.obj");
//	Transform *t5 = new Transform(rotate_y(180));
//	if (mesh2 != nullptr) {
//		auto gp5 = new GeometryPrimitive(mesh2, material, nullptr);
//		primitives.push_back(gp5);
//	}
//
	//auto mesh3 = ModelIO::load("../Models/obj/mitsuba/mitsuba-sphere.obj");
	Transform *t6 = new Transform(scale(1.f, 1.f, 1.f));
	Transform *it6 = new Transform(inverse(*t6));
	auto mesh3 = new TriangleMesh(t6, it6, false, R"(../Models/obj/mitsuba/mitsuba-sphere.obj)", nullptr);
	if (mesh3 != nullptr) {
		auto gp6 = new GeometryPrimitive(mesh3, material, nullptr);
		primitives.push_back(gp6);
	}

	//auto box = new Box(10, 10, 10);
	//auto gp7 = new GeometryPrimitive(box, material, nullptr);
	//primitives.push_back(gp7);
	//auto ground = ModelIO::load("../Models/plane.obj");
	Transform *t7 = new Transform(scale(10, 10, 10));
	Transform *it7 = new Transform(inverse(*t7));
	auto ground = new TriangleMesh(t7, it7, false, R"(../Models/plane.obj)", nullptr);
	if (ground != nullptr) {
		auto gp7 = new GeometryPrimitive(ground, material, nullptr);
		primitives.push_back(gp7);
	}

	kdTree = new KdTree(primitives, 80, 1, 0.5f, 1, -1);
}
