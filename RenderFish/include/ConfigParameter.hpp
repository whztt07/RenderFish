#pragma once
#include "RenderFish.hpp"
#include "tinyxml2.h"
#include <string>
#include <map>
#include <vector>
#include <io.h>

using namespace std;

struct ConfigParmeter {
	vector<ConfigParmeter*> children;
	map<string, string> attribute;
};

typedef map<string, ConfigParmeter> Config;

static Config load_config_xml_file(const string& file_path)
{
	Config config;
	if (_access(file_path.c_str(), 0) != 0) {
		error("can not open file %s.\n", file_path.c_str());
	}
	tinyxml2::XMLDocument doc;
	doc.LoadFile(file_path.c_str());

	const char* title = doc.FirstChild()->Value();
	info("%s\n", title);

	return config;
}
