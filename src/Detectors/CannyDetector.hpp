#pragma once
#include "EdgeDetector.hpp"
#include "../Misc/Base.hpp"

namespace tr
{
	class CannyDetector : public EdgeDetector
	{
		virtual void detectEdges(const cv::Mat& img, std::vector<Edge<vec2f>>& edges) const override;
	};
}
