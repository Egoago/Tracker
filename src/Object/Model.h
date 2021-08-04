#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <opencv2/core/mat.hpp>
#include "../Misc/ConfigParser.h"
#include "../Coordinates.h"
#include "../Math/Tensor.h"

namespace tr {
	class Model {
		static ConfigParser config;
		std::string objectName;
		Tensor<Template> templates;

		//Convenience declarations
		struct Candidate {
			glm::vec3 pos, dir;
			Candidate(glm::vec3 pos, glm::vec3 dir) : pos(pos), dir(dir) {}
			Candidate(cv::Point3f pos, cv::Point3f dir) :
				pos(pos.x, pos.y, pos.z),
				dir(dir.x, dir.y, dir.z) {}
		};

		//Temp buffers
		std::vector<Candidate> candidates;
		std::vector<cv::Mat*> textureMaps; //TODO smart pointer

		void generarteObject(const std::string& fileName);
		void generate6DOFs();
		bool load();
		void extractCandidates();
		void rasterizeCandidates(Template* temp);
	public:
		Model(std::string name);
		~Model() {}

		void save(std::string fileName = "");

		void setName(const char* str) { objectName = str; }

		Tensor<Template>& getTemplates() { return templates; }

		friend std::ostream& operator<<(std::ostream& ost, const Model& model) {
			ost << model.templates;
			return ost;
		}
	};
}