#pragma once
#include "../Coordinates.h"
#include <opencv2/core/mat.hpp>
#include "Estimator.h"
#include "Registrator.h"
#include "../Math/Tensor.h"
#include "../Misc/ConfigParser.h"
#include "../DistanceTensor.h"

namespace tr
{
	class PoseEstimator
	{
		static ConfigParser config;
		DistanceTensor distanceTensor;
		Estimator* estimator = nullptr;
		Registrator* registrator = nullptr;
	public:
		PoseEstimator(const int width, const int height, Tensor<Template>& templates);
		~PoseEstimator();
		SixDOF getPose(const cv::Mat& frame);
	};

}
