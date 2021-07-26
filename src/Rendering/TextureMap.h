#pragma once
#include <GL/glew.h>
#include <opencv2/core/mat.hpp>
#include <glm/ext/vector_uint2.hpp>

class TextureMap : public cv::Mat {
	int cvType;
	unsigned int glBuffer;
	
public:
	TextureMap(int cvType, glm::uvec2 resolution);
	~TextureMap();
	cv::Mat* copy();
	GLenum bind(unsigned int index);
};

