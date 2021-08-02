#pragma once
#include "EdgeDetector.h"

namespace tr
{
	class LSDDetector : public EdgeDetector
	{
	private:
		float scale;
	public:

		LSDDetector(float scale = 0.8f) :scale(scale) {}

		virtual void detectEdges(const cv::Mat& img, std::vector<Edge<glm::vec2>>& edges) const override;
	};
}

