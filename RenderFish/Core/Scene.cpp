#include "Scene.hpp"
#include "Light.hpp"

Scene::~Scene()
{
	delete aggregate;
	for (Light* l : lights)
		delete l;
}