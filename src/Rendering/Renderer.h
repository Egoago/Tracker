#pragma once
#include "Shader.h"
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>
#include "../Object/Geometry.h"
#include "../Object/Coordinates.h"

class Renderer
{
private:
	enum Pipeline {
		POS = 0, MASK, DIR
	};

	glm::mat4 ProjMtx, ViewModelMtx;
	GLuint frameBuffers[3];
	GLuint mapBuffers[3];
	GLuint VAOs[3];
	unsigned int faceCount, highEgdeCount, lowEdgeCount;
	Shader *faceShader, *edgeShader;
	glm::uvec2 resolution;
	void createFrameBuffers();
	const float highThreshold, lowThreshold;
	float nearP, farP;
public:
	Renderer(float highThreshold, float lowThreshold, unsigned int width = 1000, unsigned int height = 1000);
	~Renderer();
	void setGeometry(const Geometry& geometry);
	void setProj(float fov = 45.0f, float nearP = 200.0f, float farP = 5000.0f);
	const glm::mat4 getMVP() const { return ProjMtx * ViewModelMtx; }
	void setModel(SixDOF& sixDOF);
	glm::mat4 render(void* posMap, void* maskMap, void* dirMap);
};

