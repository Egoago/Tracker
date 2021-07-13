#pragma once
#include "Misc/ConfigParser.h"
#include <opencv2/core/mat.hpp>
#include "Detectors/EdgeDetector.h"

//TODO rename + reorganise file hierarchy
class DCDT3Generator
{
private:
	static ConfigParser config;
	const size_t q;
	std::vector<cv::Mat> dcdt3;
	const EdgeDetector* edgeDetector;

	float quantize(float value) const;
	int quantizedIndex(float value) const;
	void directedDistanceTransform();
public:
	DCDT3Generator();
	~DCDT3Generator() {
		delete edgeDetector;
	};
	inline const std::vector<cv::Mat>& getDCDT3() const { return dcdt3; };
	void setFrame(cv::Mat& nextFrame);
};

