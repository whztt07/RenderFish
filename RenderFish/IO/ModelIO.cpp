#include "ModelIO.hpp"
#include <fstream>
#include "Vector.hpp"
#include "ModelProcessing.hpp"

string get_file_type(const string & name)
{
	size_t i;
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
	
	////http://en.cppreference.com/w/cpp/language/operator_precedence
	//// the precedence of -- is prior than *
	//--(*vertex_id);
	//--(*uv_id);
	//--(*normal_id);
}


bool ModelIO::load(const string& path, TriangleMesh * p_triangle_mesh)
{
	std::ifstream in(path.c_str());

	if (!in.good())
	{
		error("file not exists: %s", path.c_str());
		return false;
	}
	
	Assert(p_triangle_mesh != nullptr);

	string ext = get_file_type(path);
	if (ext == "obj")
		return load_obj(path, in, p_triangle_mesh);
	else
		error("Unsupported model file type: %s\n", ext.c_str());
	return false;
}

bool ModelIO::load_obj(const string& path, std::ifstream& fin, TriangleMesh * p_triangle_mesh)
{
	char buffer[256];
	char str[256];
	float f1, f2, f3;

	int n_triangles = 0;
	int n_vertices = 0;
	vector<Point> positions;
	vector<Normal> normals;
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
				positions.push_back(Point(f1, f2, f3));
				//info("%f %f %f\n", f1, f2, f3);
				n_vertices++;
			}
			else {
				error("vertex not in wanted format in load_obj at line: %d\n", line_number);
				return false;
			}
		}
		else if(buffer[0] == 'v' && (buffer[1] == 't')) {
			if (sscanf_s(buffer, "vt %f %f", &f1, &f2) == 2) {
				uvs.push_back(Vec2(f1, f2));
			}
			else {
				error("vertex not in wanted format in load_obj at line: %d\n", line_number);
				return false;
			}
		}
		else if (buffer[0] == 'v' && (buffer[1] == 'n')) {
			if (sscanf_s(buffer, "vn %f %f %f", &f1, &f2, &f3) == 3) {
				normals.push_back(Normal(f1, f2, f3));
			}
			else {
				error("vertex not in wanted format in load_obj at line: %d\n", line_number);
				return false;
			}
		}
		else if(buffer[0] == 'f' && buffer[1] == ' ') {
			if (sscanf_s(buffer, "f %s %s %s", face, 32, face+1, 32, face+2, 32) == 3) {
				int v, u, n;
				//info("%s %s %s\n", face, face + 1, face + 2);
				for (int i = 0; i < 3; i++)
				{
					split_string(face[i], &v, &u, &n);
					--v, --u, --n;
					if (v < 0) v = positions.size() + v + 1;
					if (u < 0) u = uvs.size() + u + 1;
					if (n < 0) n = normals.size() + n + 1;
					if (v < 0 || u < 0 || n < 0) {
						error("    face index error [load_obj] at line: %d\n", line_number);
						return false;
					}
					p_index.push_back(v);
					uv_index.push_back(u);
					n_index.push_back(n);
				}
				n_triangles++;
			}
			else {
				error("vertex not in wanted format in load_obj at line: %d\n", line_number);
				return false;
			}
		}
	}

	Assert(positions.size() == n_vertices);
	Assert(p_index.size() == n_triangles * 3);

	if (n_index.size() != n_triangles * 3) {
		//error("some vertices do not have normal!\n");
		info("some vertices do not have normal. Generate normals...\n");
		ModelProcessing::Mesh mesh(positions, p_index);
		mesh.generate_normals(&normals);
		n_index.clear();
		n_index.reserve(n_triangles * 3);
		for (int i = 0; i < n_triangles * 3; ++i) {
			n_index.push_back(p_index[i]);
		}
		//return nullptr;
	}

	//if (!normals.empty()) {
		//Assert(n_index.size() == n_triangles * 3);
		if (n_index.size() != n_triangles * 3) {
			error("some vertices do not have normal!\n");
			return false;
		}
	//}
	//if (!uvs.empty()) {
		//Assert(uv_index.size() == n_triangles * 3);
		if (uv_index.size() != n_triangles * 3) {
			error("some vertices do not have uv!\n");
			return false;
		}
	//}

	info("Model loading finished. n_triangles = %d, n_vertices = %d.\n", n_triangles, n_vertices);
	
	vector<Point> new_points;
	vector<Vec2> new_uvs;
	vector<Normal> new_normals;
	vector<int> uv_id_at_p(n_vertices, -1);

	new_uvs.resize(n_vertices);
	new_normals.resize(n_vertices);
	uv_id_at_p.resize(n_vertices);
	for (int i = 0; i < n_triangles; i++) {
		int v = i * 3;
		for (int j = 0; j < 3; ++j) {
			int id_at_p_array = p_index[v];
			if (uv_id_at_p[id_at_p_array] == -1) {
				uv_id_at_p[id_at_p_array] = uv_index[v];
				new_uvs[id_at_p_array] = uvs[uv_index[v]];
				new_normals[id_at_p_array] = normals[n_index[v]];
			}
			else if (uv_id_at_p[id_at_p_array] != uv_index[v]) {	// this vertex has other uv already
				int new_id = (int)positions.size();
				positions.push_back(positions[p_index[v]]);				// add a new vertex
				new_uvs.push_back(uvs[uv_index[v]]);
				new_normals.push_back(normals[n_index[v]]);
				p_index[v] = new_id;
			}
			v++;
		}
	}

	info("Model processing finished. Add %d new vertices. %d vertices in all\n", positions.size() - n_vertices, positions.size());
	n_vertices = (int)positions.size();
	//return new TriangleMesh(&Transform::identity, &Transform::identity, false, n_triangles, n_vertices,
	//	&p_index[0], &positions[0], &new_normals[0], nullptr, &new_uvs[0], nullptr);
	p_triangle_mesh->_set_data(n_triangles, n_vertices, &p_index[0], &positions[0], &new_normals[0], nullptr, &new_uvs[0]);
	
	return true;
}