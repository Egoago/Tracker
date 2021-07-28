#pragma once
#include "Misc/ConfigParser.h"
#include <opencv2/core/mat.hpp>
#include "Detectors/EdgeDetector.h"

//TODO rename + reorganise file hierarchy
class DCDT3Generator
{
private:
	static ConfigParser config;
	const unsigned int q;
	std::vector<cv::Mat>& dcdt3;
	const EdgeDetector* edgeDetector;

	//temp buffers
	std::vector<std::vector<tr::Edge<glm::vec2>>> quantizedEdges;
	std::vector<cv::Mat>& other;
	std::vector<cv::Mat> buffer1, buffer2;
	bool first;
	cv::Mat tmp;
	float* costs;

	void directedDistanceTransform();
	void gaussianBlur();
	void swapBuffers();
public:
	DCDT3Generator(unsigned int width, unsigned int height);
	~DCDT3Generator() {
		delete edgeDetector;
		delete[] costs;
	};
	inline const std::vector<cv::Mat>& getDCDT3() const { return dcdt3; };
	std::vector<cv::Mat>& setFrame(cv::Mat& nextFrame);
};

