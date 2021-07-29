#include "TextureMap.h"
#include <iostream>
//TODO clear glew

using namespace tr;

TextureMap::TextureMap(int cvType, glm::uvec2 resolution) :
	cvType(cvType),
	cv::Mat(cv::Size(resolution.x, resolution.y), cvType) {
	glGenTextures(1, &glBuffer);
	glBindTexture(GL_TEXTURE_2D, glBuffer);
	switch (cvType) {
	case CV_32FC3:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, resolution.x, resolution.y, 0, GL_RGB, GL_FLOAT, 0);
		break;
	case CV_8U:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, resolution.x, resolution.y, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
		break;
	case CV_32F:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, resolution.x, resolution.y, 0, GL_RED, GL_FLOAT, 0);
		break;
	default: {
		std::cerr << "not supported texture type: " << cvType << std::endl;
		exit(1);
	}
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

TextureMap::~TextureMap()
{
	//TODO implement
	std::cout << "deleting pipeline\n";
}

cv::Mat* TextureMap::copy() {
	glBindTexture(GL_TEXTURE_2D, glBuffer);
	switch (cvType) {
	case CV_32FC3:
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, data);
		break;
	case CV_8U:
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_BYTE, data);
		break;
	case CV_32F:
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, data);
		break;
	default: {
		std::cerr << "not supported texture type: " << cvType << std::endl;
		exit(1);
	}
	}
	return this;
}

GLenum TextureMap::bind(unsigned int index)
{
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, glBuffer, 0);
	return GL_COLOR_ATTACHMENT0 + index;
}
