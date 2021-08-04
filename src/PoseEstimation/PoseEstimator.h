#pragma once
#include "../Coordinates.h"
#include <opencv2/core/mat.hpp>
#include <memory>
#include "Estimator.h"
#include "Registrator.h"
#include "../Math/Tensor.h"
#include "../Misc/ConfigParser.h"
#include "../DistanceTensor.h"

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
