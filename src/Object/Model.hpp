#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <opencv2/core/mat.hpp>
#include "../Misc/ConfigParser.hpp"
#include "../Misc/Base.hpp"
#include "../Math/Template.hpp"
#include "../Math/Tensor.hpp"

namespace tr {
	class Model {
		static ConfigParser config;
		std::string objectName;
		Tensor<Template> templates;
		mat4f P;

		//Convenience declarations
		struct Candidate {
			vec3f pos, dir;
			Candidate(vec3f pos, vec3f dir) : pos(pos), dir(dir) {}
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
		void rasterizeCandidates(Template* temp, const mat4f& mvp);
	public:
		Model(std::string name);
		~Model() {}

		void save(std::string fileName = "");

		void setName(const char* str) { objectName = str; }

		inline mat4f getP() { return P; }

		Tensor<Template>& getTemplates() { return templates; }

		friend std::ostream& operator<<(std::ostream& ost, const Model& model) {
			ost << model.templates;
			return ost;
		}
	};
}