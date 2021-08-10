#pragma once
#include <vector>
#include <opencv2/core/mat.hpp>
#include <glm/vec2.hpp>
#include "../Math/Edge.hpp"

namespace tr {
	class EdgeDetector {
	public:
		virtual void detectEdges(const cv::Mat& img, std::vector<Edge<glm::vec2>>& edges) const = 0;
	};
}

