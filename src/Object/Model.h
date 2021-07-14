#pragma once
#include "../Misc/ConfigParser.h"
#include "Coordinates.h"
#include "boost/multi_array.hpp"

//TODO remove monitoring
#include <iostream>

class Model
{
	static ConfigParser config;
	std::string objectName;
	size_t dimensions[6];
	boost::multi_array<Template, 6> templates;
	
	void generarteObject(const std::string& fileName);
	void generate6DOFs();
	void allocateRegistry();
	void rasterize(const std::vector<Edge<>>& edges, Template* templates);
	void load();
public:

	Model(std::string name);
	~Model() {}
	
	void save(std::string fileName = "");

	void setName(const char* str) { objectName = str; }

	auto& getTemplates() const { return templates; }
	
	void print() const {
		std::cout << "dimensions:";
		for (int i = 0; i < templates.dimensionality; i++)
			std::cout << " " << dimensions[i];
		std::cout << std::endl;
		for (const Template* i = templates.data(); i < (templates.data() + templates.num_elements()); i++)
			i->sixDOF.print(std::cout);
	}
};