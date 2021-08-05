#pragma once
#include "../Coordinates.hpp"
#include <opencv2/core/mat.hpp>
#include <memory>
#include "Estimator.hpp"
#include "Registrator.hpp"
#include "../Math/Tensor.hpp"
#include "../Misc/ConfigParser.hpp"
#include "DistanceTensor.hpp"

namespace tr {
	class PoseEstimator {
		static ConfigParser config;
		DistanceTensor distanceTensor;
		std::unique_ptr<Estimator> estimator;
		std::unique_ptr<Registrator> registrator;
	public:
		PoseEstimator(const int width, const int height, Tensor<Template>& templates);
		SixDOF getPose(const cv::Mat& frame);
	};

}
