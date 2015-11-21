#define MS_NO_COREDLL
#define Py_NO_ENABLE_SHARED
#include <Python.h>
#include <iostream>

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
	auto x = great_function_from_python(1);
	std::cout << x << std::endl;
	return 0;
}
