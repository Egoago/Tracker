#pragma once
#include <glm/mat4x4.hpp>
#include "../Misc/ConfigParser.h"
#include "../Object/Geometry.h"
#include "../Object/Coordinates.h"
#include "Pipeline.h"

class Renderer
{
private:
	static ConfigParser config;

	std::vector<Pipeline*> pipelines;
	std::vector<TextureMap*> textureMaps;

	glm::mat4 ProjMtx, ViewModelMtx;

	void updatePipelines();
	void readConfig();
	float nearP, farP, fov, aspect;
	glm::uvec2 resolution;
	void setGeometry(const Geometry& geometry);
public:
	Renderer(const Geometry& geometry);
	~Renderer();
	void setProj(float fov, float nearP, float farP, float aspect);
	void setModel(SixDOF& sixDOF);
	glm::uvec2 getResolution() { return resolution; }
	std::vector<cv::Mat*> render();
};

