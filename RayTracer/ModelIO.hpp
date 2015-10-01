#pragma once
#include "RenderFish.hpp"
#include "TriangleMesh.hpp"

string get_file_type(const string& name);

class ModelIO
{
public:
	static void load(const string& path, TriangleMesh* mesh);

	static void load_obj(const string& path, std::ifstream& fin, TriangleMesh* mesh);


private:
	ModelIO();
};

