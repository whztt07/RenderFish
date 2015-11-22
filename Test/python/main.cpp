#define MS_NO_COREDLL
#define Py_NO_ENABLE_SHARED
#include <Python.h>
#include <iostream>
#include <string>
using namespace std;

void parse_xml(string xml_file_name) {
	PyObject *pModule, *pFunc, *pArgs, *pValue;
	string python_module_name = "parse_xml";
	pModule = PyImport_Import(PyString_FromString(python_module_name.c_str()));
	pFunc = PyObject_GetAttrString(pModule, "parse_xml");
	pArgs = PyTuple_New(1);
	PyTuple_SetItem(pArgs, 0, PyString_FromString(xml_file_name.c_str()));
	pValue = PyObject_CallObject(pFunc, pArgs);
}

int great_function_from_python(int a) {
	int res;
	PyObject *pModule, *pFunc, *pArgs, *pValue;

	// import
	pModule = PyImport_Import(PyString_FromString("test01"));
	// test01.great_function
	pFunc = PyObject_GetAttrString(pModule, "great_function");
	// build args
	pArgs = PyTuple_New(1);
	PyTuple_SetItem(pArgs, 0, PyInt_FromLong(a));
	// call
	pValue = PyObject_CallObject(pFunc, pArgs);

	res = PyInt_AsLong(pValue);

	return res;
}

int main(int argc, char *argv[])
{
	Py_Initialize();
	//auto x = great_function_from_python(1);
	parse_xml("scene02.xml");
	//std::cout << x << std::endl;
	Py_Finalize();
	return 0;
}
