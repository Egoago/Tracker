#pragma once
#include <opencv2/core/mat.hpp>
#include "../Misc/ConfigParser.h"
#include "Coordinates.h"
#include "boost/multi_array.hpp"

//TODO remove monitoring
#include <iostream>

class Model
{
	static ConfigParser config;
	std::string objectName;
	unsigned int dimensions[6];
	boost::multi_array<tr::Template, 6> templates;

	//Convenience declarations
	enum TextureMapIndex {
		HPOS = 0,
		HDIR = 1,
		LPOS = 2,
		LDIR = 3
	};
	struct Candidate {
		glm::vec3 pos, dir;
		Candidate(glm::vec3 pos, glm::vec3 dir) : pos(pos), dir(dir) {}
		Candidate(cv::Point3f pos, cv::Point3f dir) :
			pos(pos.x, pos.y, pos.z),
			dir(dir.x, dir.y, dir.z) {}
	};

	//Temp buffers
	std::vector<Candidate> candidates;
	std::vector<cv::Mat*> textureMaps;
	
	void generarteObject(const std::string& fileName);
	void generate6DOFs();
	void allocateRegistry();
	void load();
	void extractCandidates(const glm::mat4& MVP);
	void rasterizeCandidates(tr::Template* temp);
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
		for (const tr::Template* i = templates.data(); i < (templates.data() + templates.num_elements()); i++)
			i->sixDOF.print(std::cout);
	}
};