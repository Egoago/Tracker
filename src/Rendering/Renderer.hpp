#pragma once
#include <glm/mat4x4.hpp>
#include <opencv2/core/mat.hpp>
#include <vector>
#include <memory>
#include "../Math/SixDOF.hpp"
#include "../Misc/ConfigParser.hpp"
#include "../Misc/Base.hpp"
#include "../Object/Geometry.hpp"
#include "TextureMap.hpp"
#include "Pipeline.hpp"

namespace tr {
	class Renderer {
	private:
		static ConfigParser config;

		std::vector<std::unique_ptr<Pipeline>> pipelines;
		std::vector<std::shared_ptr<TextureMap>> textureMaps;
		unsigned int depthBuffer;
		int glutWindow;

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
		void setVM(const glm::mat4& MV);
		inline glm::mat4 getPVM() const { return ProjMtx * ViewModelMtx; }
		inline glm::mat4 getVM() const { return ViewModelMtx; }
		inline glm::mat4 getP() const { return ProjMtx; }
		inline glm::uvec2 getResolution() { return resolution; }
		void render();
		std::vector<cv::Mat*>getTextures();
	};
}

