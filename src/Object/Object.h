#pragma once
#include "../Misc/ConfigParser.h"
#include "Coordinates.h"
#include "boost/multi_array.hpp"
#include <iostream>

class Object
{
	static ConfigParser config;
	std::string objectName;
	typedef boost::multi_array<Snapshot, 6> Registry;
	size_t dimensions[6];
	boost::array<Registry::index, 3> shape = { { 3, 4, 2 } };
	Registry snapshots;

	void generarteObject(const std::string& fileName);
	void generate6DOFs();
	void allocateRegistry();
	void rasterize(const std::vector<Edge>& edges, Snapshot* snapshot);
public:

	Object(std::string name);
	~Object() {}

	void save(std::string fileName = "");
	void load();

	void setName(const char* str) { objectName = str; }

	void print() {
		std::cout << "dimensions:";
		for (int i = 0; i < snapshots.dimensionality; i++)
			std::cout << " " << dimensions[i];
		std::cout << std::endl;
		for (Snapshot* i = snapshots.data(); i < (snapshots.data() + snapshots.num_elements()); i++)
			i->sixDOF.print(std::cout);
	}
};