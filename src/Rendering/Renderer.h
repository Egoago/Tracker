#pragma once
#include <glm/mat4x4.hpp>
#include <opencv2/core/mat.hpp>
#include <vector>
#include "../Misc/ConfigParser.h"
#include "../Object/Geometry.h"
#include "../Coordinates.h"
#include "TextureMap.h"
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

		enum TextureMapIndex {
			MESH = 0,
			HPOS = 1,
			HDIR = 2,
			LPOS = 3,
			LDIR = 4
		};

		Renderer(const Geometry& geometry);
		~Renderer();
		void setProj(float fov, float nearP, float farP, float aspect);
		void setModel(SixDOF& sixDOF);
		glm::mat4 getMVP() const { return ProjMtx * ViewModelMtx; }
		glm::mat4 getVM() const { return ViewModelMtx; }
		glm::uvec2 getResolution() { return resolution; }
		void render();
		std::vector<cv::Mat*>getTextures();
	};
}

