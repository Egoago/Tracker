#include "TextureMap.hpp"
#include <iostream>
#include "../Misc/Log.hpp"
#include <GL/glew.h>

using namespace tr;

TextureMap::TextureMap(int cvType, uvec2 resolution) :
	cvType(cvType),
	cv::Mat(cv::Size(resolution.x(), resolution.y()), cvType) {
	glGenTextures(1, &glBuffer);
	glBindTexture(GL_TEXTURE_2D, glBuffer);
	switch (cvType) {
	case CV_32FC3:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, resolution.x(), resolution.y(), 0, GL_RGB, GL_FLOAT, 0);
		break;
	case CV_8U:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, resolution.x(), resolution.y(), 0, GL_RED, GL_UNSIGNED_BYTE, 0);
		break;
	case CV_32F:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, resolution.x(), resolution.y(), 0, GL_RED, GL_FLOAT, 0);
		break;
	default: {
			Logger::error("not supported texture type: " + std::to_string(cvType));
			exit(1);
		}
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

TextureMap::~TextureMap() {
	glDeleteTextures(1, &glBuffer);
}

cv::Mat* TextureMap::copyToCPU() {
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
			Logger::error("not supported texture type: " + std::to_string(cvType));
			exit(1);
		}
	}
	return this;
}

unsigned int TextureMap::bind(unsigned int index) {
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, glBuffer, 0);
	return GL_COLOR_ATTACHMENT0 + index;
}
