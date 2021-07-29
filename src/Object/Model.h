#pragma once
#include <opencv2/core/mat.hpp>
#include "../Misc/ConfigParser.h"
#include "Coordinates.h"
#include "boost/multi_array.hpp"

//TODO remove monitoring
#include <iostream>

namespace tr
{
	class Model
	{
		static ConfigParser config;
		std::string objectName;
		unsigned int dimensions[6];
		boost::multi_array<Template, 6> templates;

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
		bool load();
		void extractCandidates();
		void rasterizeCandidates(Template* temp);
	public:
		Model(std::string name);
		~Model() {}

		void save(std::string fileName = "");

		void setName(const char* str) { objectName = str; }

		auto& getTemplates() const { return templates; }

		friend std::ostream& operator<<(std::ostream& ost, const Model& model) {
			ost << "dimensions:";
			for (int i = 0; i < model.templates.dimensionality; i++)
				ost << " " << model.dimensions[i];
			ost << std::endl;
			for (const Template* i = model.templates.data(); i < (model.templates.data() + model.templates.num_elements()); i++)
				ost << *i;
			return ost;
		}
	};
}