#pragma once
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

		struct CameraCalibration {
			float nearPlane, farPlane, FOV, aspect;
			uvec2 resolution;
		};

		//TODO simplify to one matrix
		Eigen::Matrix<float, 4, 4> ProjMtx, ViewModelMtx;

		void updatePipelines();
		static CameraCalibration readConfig();
		float nearP, farP;
		uvec2 resolution;
		void setGeometry(const Geometry& geometry);
	public:
		static mat4f getDefaultP();

		enum TextureMapIndex {
			MESH = 0,
			HPOS = 1,
			HDIR = 2,
			LPOS = 3,
			LDIR = 4
		};

		Renderer(const Geometry& geometry);
		~Renderer();
		void setProj(float fov, float aspect, float nearP, float farP);
		void setProj(const mat4f& P);
		void setVM(const mat4f& MV);
		inline mat4f getPVM() const { return ProjMtx * ViewModelMtx; }
		inline mat4f getVM() const { return ViewModelMtx; }
		inline uvec2 getResolution() { return resolution; }
		void render();
		std::vector<cv::Mat*>getTextures();
	};
}

