#pragma once
#include "../Misc/Base.hpp"
#include <vector>
#include <opencv2/core/mat.hpp>

namespace tr {
	class MultiFlashDetector {
		const uint flashCount;

		template <typename T>
		using DinArr = Eigen::Array<T, -1, -1, Eigen::RowMajor>;
		typedef DinArr<float> FloatMat;

		FloatMat *R, *I;
		FloatMat ambient, Imax;
	public:
		MultiFlashDetector(const uint flashCount);
		~MultiFlashDetector();
		cv::Mat getDepthMap(const std::vector<cv::Mat>& flashImages);
	};	
}

