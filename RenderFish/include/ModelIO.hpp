#pragma once
#include "RenderFish.hpp"
#include "TriangleMesh.hpp"

string get_file_type(const string& name);

// static class for loading 3d model file from files;
class ModelIO
{
public:
	static bool load(const string& path, TriangleMesh * p_triangle_mesh);
	static bool load_obj(const string& path, std::ifstream& fin, TriangleMesh * p_triangle_mesh);

private:
	ModelIO();
	ModelIO(const ModelIO&);
	ModelIO& operator=(const ModelIO&);
};

