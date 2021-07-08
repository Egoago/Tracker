#pragma once
#include "../Misc/ConfigParser.h"

class Object
{
	static ConfigParser config;
	std::string objectName;

	void loadObject(const std::string& objectName);
	void generarteObject(const std::string& fileName);
public:

	Object() {}
	Object(std::string name);
	~Object() {}
};