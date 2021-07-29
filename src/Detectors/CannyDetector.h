#pragma once
#include "EdgeDetector.h"

namespace tr
{
	class CannyDetector : public EdgeDetector
	{
		virtual void detectEdges(cv::Mat& img, std::vector<Edge<glm::vec2>>& edges) const override;
	};
}
