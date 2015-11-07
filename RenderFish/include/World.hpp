#pragma once

#include <vector>
#include "Color.hpp"
#include "Window.hpp"
#include "DifferentialGeometry.hpp"
#include "KdTree.hpp"
#include "MatteMaterial.hpp"

class Tracer;

class World
{
public:
	vector<Reference<Primitive>> primitives;
	KdTree* kdTree;

	MatteMaterial material;

	void build(int width, int height);
};