#pragma once
#include "Shader.h"
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>
#include "../Object/Geometry.h"

class Renderer
{
private:
	glm::mat4 Proj, Model;
	GLuint frameBuffer, colorBuffer, depthBuffer;
	Shader shader;
	glm::uvec2 resolution;
	void createFrameBuffer();
public:
	Renderer(unsigned int width = 1000, unsigned int height = 1000);
	void setProj(float fov = 45.0f, float nearP = 1.0f, float farP = 10000.0f);
	void setModel(SixDOF sixDOF);
	glm::mat4 renderModel(Geometry& geometry, unsigned char* depthMap, unsigned char* colorMap);
};

