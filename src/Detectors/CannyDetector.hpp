#pragma once
#include "EdgeDetector.hpp"

namespace tr
{
	class CannyDetector : public EdgeDetector
	{
		virtual void detectEdges(const cv::Mat& img, std::vector<Edge<glm::vec2>>& edges) const override;
	};
}
