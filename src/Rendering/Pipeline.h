#pragma once
#include <vector>
#include "TextureMap.h"
#include "Shader.h"

class Pipeline {
	GLuint frameBuffer;
	Shader* shader;
	std::vector<TextureMap*> textureMaps;
	GLbitfield clearMask;
	GLenum drawPrimitive;
	GLuint VAO;
	unsigned int primitiveCount;
public:
	inline void setGeometry(GLuint vao, unsigned int primitiveCount) {
		VAO = vao;
		primitiveCount = primitiveCount;
	}

	Shader* getShader() { return shader; }
	Pipeline(
		const char* name,
		const std::vector<TextureMap*>& textureMaps,
		GLenum drawPrimitive,
		GLbitfield clearMask,
		GLuint depthBuffer);
	~Pipeline();
	void render(std::vector<cv::Mat*>& outTextures);
};

