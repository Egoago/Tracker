#pragma once
#include <vector>
#include "../Object/Coordinates.h"
#include <opencv2/core/mat.hpp>
#include <glm/vec2.hpp>

namespace tr
{
	class EdgeDetector
	{
	public:
		virtual void detectEdges(cv::Mat& img, std::vector<Edge<glm::vec2>>& edges) const = 0;
	};
}

