#pragma once
#include "../Object/Coordinates.h"
#include <opencv2/core/mat.hpp>
#include "../DCDT3Generator.h"
#include "../Object/Model.h"

class PoseEstimator
{
	DCDT3Generator generator;
	const Model& model;
public:
	PoseEstimator(const int width, const int height, const Model& model);
	tr::SixDOF getPose(cv::Mat& frame);
};

