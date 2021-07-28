#pragma once
#include "EdgeDetector.h"
class CannyDetector : public EdgeDetector
{
	virtual void detectEdges(cv::Mat& img, std::vector<tr::Edge<glm::vec2>>& edges) const override;
};
