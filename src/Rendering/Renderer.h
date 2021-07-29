#pragma once
#include <glm/mat4x4.hpp>
#include "../Misc/ConfigParser.h"
#include "../Object/Geometry.h"
#include "../Object/Coordinates.h"
#include "Pipeline.h"

namespace tr
{
	class Renderer
	{
	private:
		static ConfigParser config;

		std::vector<Pipeline*> pipelines;
		std::vector<TextureMap*> textureMaps;

		//TODO simplify to one matrix
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
		glm::mat4 getMVP() const { return ProjMtx * ViewModelMtx; }
		glm::mat4 getVM() const { return ViewModelMtx; }
		glm::uvec2 getResolution() { return resolution; }
		void render(std::vector<cv::Mat*>& outTextures);
	};
}

