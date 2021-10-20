#pragma once
#include <opencv2/core/mat.hpp>
#include <memory>
#include "Estimator.hpp"
#include "Registrator.hpp"
#include "../Math/Tensor.hpp"
#include "../Math/Template.hpp"
#include "../Misc/Base.hpp"
#include "DistanceTensor.hpp"

namespace tr {
	class PoseEstimator {
		DistanceTensor distanceTensor;
		std::unique_ptr<Estimator> estimator;
		std::unique_ptr<Registrator> registrator;
	public:
		PoseEstimator(const Tensor<Template>& templates, const mat4f& P, const float aspect);
		SixDOF getPose(const cv::Mat& frame);
	};

}
