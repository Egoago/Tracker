#pragma once
#include "EdgeDetector.h"

class LSDDetector : public EdgeDetector
{
private:
	float scale;
public:

	LSDDetector(float scale = 0.8f):scale(scale) {}

	virtual std::vector<Edge<glm::vec2>> detectEdges(cv::Mat& img) const override;
};

