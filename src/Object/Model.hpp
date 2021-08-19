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
		Tensor<Template> templates;
		mat4f P;

		bool load(const std::string& filename);
	public:
		Model(const std::string& fileName);
		Model(const std::string& fileName, const mat4f& P);
		~Model() {}

		void save(const std::string& fileName = "");

		inline mat4f getP() const { return P; }
		inline const Tensor<Template>& getTemplates() const { return templates; }

		inline friend std::ostream& operator<<(std::ostream& ost, const Model& model) {
			ost << model.templates;
			return ost;
		}
	};
}