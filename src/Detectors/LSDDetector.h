#pragma once
#include "EdgeDetector.h"

class LSDDetector : public EdgeDetector
{
private:
	float scale;
public:

	LSDDetector(float scale = 0.8f):scale(scale) {}

	virtual void detectEdges(cv::Mat& img, std::vector<tr::Edge<glm::vec2>>& edges) const override;
};

