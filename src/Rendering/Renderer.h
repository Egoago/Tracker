#pragma once
#include "Shader.h"
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>
#include "../Object/Geometry.h"
#include "../Object/Coordinates.h"

class Renderer
{
private:
	glm::mat4 Proj, Model, MVP;
	GLuint faceFrameBuffer, edgeFrameBuffer;
	GLuint normalMapBuffer, posMapBuffer, dirMapBuffer;
	GLuint faceVAO, edgeVAO;
	unsigned int faceCount, edgeCount;
	Shader faceShader, edgeShader;
	glm::uvec2 resolution;
	void createFrameBuffers();
	const float angleThreshold;
public:
	Renderer(float angleThreshold, unsigned int width = 1000, unsigned int height = 1000);
	~Renderer();
	void setGeometry(const Geometry& geometry);
	void setProj(float fov = 45.0f, float nearP = 1.0f, float farP = 10000.0f);
	const glm::mat4& getMVP() const { return MVP; }
	void setModel(SixDOF& sixDOF);
	glm::mat4 render(void* posMap, void* normalMap, void* dirMap);
};

