#pragma once
#include "RenderFish.hpp"
#include "TriangleMesh.hpp"

string get_file_type(const string& name);

class ModelIO
{
public:
	static TriangleMesh* load(const string& path);

	static TriangleMesh* load_obj(const string& path, std::ifstream& fin);


private:
	ModelIO();
};

