#pragma once
#include "Shader.h"
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>

class Renderer
{
private:
	glm::mat4 Proj, Model;
	GLuint frameBuffer, colorBuffer;
	Shader shader;
	glm::uvec2 resolution;
	void createFrameBuffer();
public:
	Renderer(int argc = 0, char** argv = nullptr, unsigned int width = 1000, unsigned int height = 1000);
	void setProj(float fov = 45.0f, float nearP = 1.0f, float farP = 1000.0f);
	void setModel(float x = 0.0f, float y = 0.0f, float z = -200.0f, float rotateX = 0.0f, float rotateY = 0.0f);
	void renderModel(const char* file, unsigned char* gl_texture_byte);
};

