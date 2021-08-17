#pragma once
#include "../Misc/Base.hpp"

namespace tr {
	template <uint Width, uint Height, uint FlashCount>
	class MultiFlashDetector {
		typedef Eigen::Array<uchar, Height, Width> BinaryMat;
	public:
		cv::Mat getDepthMap(const BinaryMat* flashImages[FlashCount+1]) const;
	};

	template<uint Width, uint Height, uint FlashCount>
	inline cv::Mat MultiFlashDetector<Width, Height, FlashCount>::getDepthMap(const BinaryMat* flashImages[FlashCount + 1]) const {
		const BinaryMat* ambientImage = flashImages[FlashCount];
		BinaryMat I[FlashCount];
		for (uint k = 0; k < FlashCount; k++)
			I[k] = *flashImages[k] - *ambientImage;
		BinaryMat Imax;
		for (uint k = 0; k < FlashCount; k++)
			Imax.max(I[k]);
		BinaryMat R[FlashCount];
		for (uint k = 0; k < FlashCount; k++)
			R[k] = I[k] / Imax;
		return cv::Mat();
	}
}

