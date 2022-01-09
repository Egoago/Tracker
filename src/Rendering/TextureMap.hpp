#pragma once
#include <opencv2/core/mat.hpp>
#include "../Misc/Base.hpp"

namespace tr
{
	class TextureMap : public cv::Mat {
		int cvType;
		unsigned int glBuffer;

	public:
		TextureMap(int cvType, uvec2 resolution);
		~TextureMap();
		cv::Mat* copyToCPU();
		unsigned int bind(unsigned int index);
	};
}

