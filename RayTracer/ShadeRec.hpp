#pragma once
#include "Math.hpp"
#include "Color.hpp"

// class World;
class Geometry;

class ShadeRec
{
public:

	bool hit_an_object = false;
	Vec3 local_hit_point;
	Vec3 normal;
	Color color = Color::black;
	//World& world;
	Geometry* geometry;

	//ShadeRec(World& world) : world(world) {}
	ShadeRec() {}
	~ShadeRec() {}
};

