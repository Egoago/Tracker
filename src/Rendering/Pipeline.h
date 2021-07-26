#pragma once
#include <vector>
#include "TextureMap.h"
#include "Shader.h"
#include <iostream>

class Pipeline {
	GLuint frameBuffer;
	Shader* shader;
	std::vector<TextureMap*> textureMaps;
	GLbitfield clearMask;
	GLenum drawPrimitive;
	GLuint VAO;
	unsigned int primitiveCount;
	bool drawElements;
public:
	inline void setGeometry(GLuint vao, unsigned int primitiveCount) {
		VAO = vao;
		this->primitiveCount = primitiveCount;
		std::cout << "primitives: " << primitiveCount << std::endl;
	}

	Shader* getShader() { return shader; }
	Pipeline(
		const char* name,
		const std::vector<TextureMap*>& textureMaps,
		GLenum drawPrimitive,
		GLbitfield clearMask,
		GLuint depthBuffer,
		bool drawElements);
	~Pipeline();
	void render(std::vector<cv::Mat*>& outTextures);
};

