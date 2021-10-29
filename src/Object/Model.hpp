#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <opencv2/core/mat.hpp>
#include "../Misc/Base.hpp"
#include "../Math/Template.hpp"
#include "../Math/Tensor.hpp"
#include "../Camera/CameraParameters.hpp"

namespace tr {
	class Model {
		Tensor<Template> templates;
		mat4f P;

		bool load(const std::string& filename);
	public:
		Model(const std::string& fileName, const CameraParameters cam = CameraParameters::default());
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