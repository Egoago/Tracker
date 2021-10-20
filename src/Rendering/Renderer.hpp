#pragma once
#include <opencv2/core/mat.hpp>
#include <vector>
#include <memory>
#include "../Math/SixDOF.hpp"
#include "../Misc/Base.hpp"
#include "../Object/Geometry.hpp"
#include "TextureMap.hpp"
#include "Pipeline.hpp"

namespace tr {
	class Renderer {
	private:
		std::vector<std::unique_ptr<Pipeline>> pipelines;
		std::vector<std::shared_ptr<TextureMap>> textureMaps;
		unsigned int depthBuffer;
		int glutWindow;

		struct CameraCalibration {
			float nearPlane, farPlane, FOV, aspect;
			uvec2 resolution;
		};

		CameraCalibration camCal;

		//TODO simplify to one matrix
		mat4f ProjMtx, ViewModelMtx, ScaleMtx;

		void updatePipelines();
		static CameraCalibration readConfig();
		float nearP, farP;
		void setGeometry(const Geometry& geometry);

		//Scaling
		bool scaling = true;
		vec3f geoBBxCenter;
		float geoBBxRadius;
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
		inline void setScaling(bool scaling) { this->scaling = scaling; }
		void setProj(float fov, float aspect, float nearP, float farP);
		void setProj(const mat4f& P);
		vec3f setVM(const mat4f& VM);
		inline mat4f getPVM() const { return ProjMtx * ViewModelMtx; }
		inline mat4f getVM() const { return ViewModelMtx; }
		inline uvec2 getResolution() { return camCal.resolution; }
		void render();
		std::vector<cv::Mat*>getTextures();
	};
}

