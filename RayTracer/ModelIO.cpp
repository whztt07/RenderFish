#include "ModelIO.hpp"
#include <fstream>
#include "Vector.hpp"

string get_file_type(const string & name)
{
	int i;
	for (i = name.length() - 1; i >= 0; --i)
	{
		if (name[i] == '.') break;
	}
	return name.substr(i + 1);
}

void split_string(const string & str, int *vertex_id, int *uv_id, int *normal_id)
{
	int count = 0;
	for (unsigned int i = 0; i < str.length(); i++)
	{
		if (str[i] == '/') count++;
	}
	*vertex_id = *uv_id = *normal_id = 0;
	if (count == 0)
		sscanf_s(str.c_str(), "%d", vertex_id);
	else if (count == 1)
		sscanf_s(str.c_str(), "%d/%d", vertex_id, uv_id);
	else if (count == 2)
		sscanf_s(str.c_str(), "%d/%d/%d", vertex_id, uv_id, normal_id);
	else
		error("vertex not in wanted format in load_obj: not triangle\n");
	
	//http://en.cppreference.com/w/cpp/language/operator_precedence
	// the precedence of -- is prior than *
	--(*vertex_id);
	--(*uv_id);
	--(*normal_id);
}

void ModelIO::load(const string & path, TriangleMesh* mesh)
{
	std::ifstream in(path.c_str());

	if (!in.good())
	{
		error("file not exists: %s", path.c_str());
		return;
	}

	string ext = get_file_type(path);
	if (ext == "obj")
		load_obj(path, in, mesh);
	else
		error("Unsupported model file type: %s\n", ext.c_str());
}

void ModelIO::load_obj(const string & path, std::ifstream & fin, TriangleMesh* mesh)
{
	//error("unimplemented function\n");

	char buffer[256];
	char str[256];
	float f1, f2, f3;

	int n_triangles = 0;
	int n_vertices = 0;
	vector<Point> verts;
	vector<Vec3> normals;
	vector<Vec2> uvs;
	vector<int> p_index;
	vector<int> uv_index;
	vector<int> n_index;

	char face[3][32];

	int line_number = 0;

	info("Loading model %s\n", path.c_str());

	while (!fin.getline(buffer, 255).eof())
	{
		buffer[255] = '\0';
		sscanf_s(buffer, "%s", str, 255);
		//info(buffer);
		line_number++;

		// vertex
		if ((buffer[0] == 'v') && (buffer[1] == ' ' || buffer[1] == 32)) {
			if (sscanf_s(buffer, "v %f %f %f", &f1, &f2, &f3) == 3) {
				verts.push_back(Point(f1, f2, f3));
				//info("%f %f %f\n", f1, f2, f3);
				n_vertices++;
			}
			else {
				error("vertex not in wanted format in load_obj at line: %d\n", line_number);
				return;
			}
		}
		else if(buffer[0] == 'v' && (buffer[1] == 't')) {
			if (sscanf_s(buffer, "vt %f %f", &f1, &f2) == 2) {
				uvs.push_back(Vec2(f1, f2));
			}
			else {
				error("vertex not in wanted format in load_obj at line: %d\n", line_number);
				return;
			}
		}
		else if (buffer[0] == 'v' && (buffer[1] == 'n')) {
			if (sscanf_s(buffer, "vn %f %f %f", &f1, &f2, &f3) == 3) {
				normals.push_back(Vec3(f1, f2, f3));
			}
			else {
				error("vertex not in wanted format in load_obj at line: %d\n", line_number);
				return;
			}
		}
		else if(buffer[0] == 'f' && buffer[1] == ' ') {
			if (sscanf_s(buffer, "f %s %s %s", face, 32, face+1, 32, face+2, 32) == 3) {
				int v, u, n;
				//info("%s %s %s\n", face, face + 1, face + 2);
				for (int i = 0; i < 3; i++)
				{
					split_string(face[i], &v, &u, &n);
					if (v < 0) {
						error("    load_obj at line: %d\n", line_number);
						return;
					}
					p_index.push_back(v);
					if (u >= 0) uv_index.push_back(u);
					if (n >= 0) n_index.push_back(n);
				}
				n_triangles++;
			}
			else {
				error("vertex not in wanted format in load_obj at line: %d\n", line_number);
				return;
			}
		}
	}

	Assert(verts.size() == n_vertices);
	Assert(p_index.size() == n_triangles * 3);
	
	info("Model loading finished. n_triangles = %d, n_vertives = %d.\n", n_triangles, n_vertices);

	mesh->n_triangles = n_triangles;
	mesh->n_vertices = n_vertices;
	mesh->position = new Point[n_vertices];
	memcpy(mesh->position, (void *)&verts[0], n_vertices * sizeof(Point));
	mesh->vertex_index = new int[n_triangles * 3];
	memcpy(mesh->vertex_index, (void*)&p_index[0], n_triangles * 3 * sizeof(int));
	//mesh->refine(mesh->refined_shapes);
}

