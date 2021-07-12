#pragma once
#include <vector>
#include "../Object/Coordinates.h"
#include <opencv2/core/mat.hpp>
#include <glm/vec2.hpp>

class EdgeDetector
{
public:
	virtual std::vector<Edge<glm::vec2>> detectEdges(cv::Mat& img) const = 0;
};

