#pragma once
#include "../Misc/ConfigParser.h"
#include "Coordinates.h"
#include "boost/multi_array.hpp"

class Object
{
	static ConfigParser config;
	std::string objectName;
	std::vector<SixDOF> sixDOFs;
	std::vector<std::vector<std::vector<std::vector<std::vector<std::vector<Snapshot>>>>>> snapshots;

	void loadObject(const std::string& objectName);
	void generarteObject(const std::string& fileName);
public:

	Object() {}
	Object(std::string name);
	~Object() {}
};