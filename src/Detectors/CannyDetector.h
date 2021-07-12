#pragma once
#include "EdgeDetector.h"
class CannyDetector : public EdgeDetector
{
	virtual std::vector<Edge<glm::vec2>> detectEdges(cv::Mat& img) const override;
};
