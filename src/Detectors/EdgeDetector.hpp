#pragma once
#include <vector>
#include <opencv2/core/mat.hpp>
#include "../Misc/Base.hpp"
#include "../Math/Edge.hpp"

namespace tr {
	class EdgeDetector {
	public:
		virtual void detectEdges(const cv::Mat& img, std::vector<Edge<vec2f>>& edges) const = 0;
	};
}

