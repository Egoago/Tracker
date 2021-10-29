#pragma once
#include <opencv2/core/mat.hpp>
#include <vector>
#include <memory>
#include "../Math/SixDOF.hpp"
#include "../Misc/Base.hpp"
#include "../Object/Geometry.hpp"
#include "../Camera/CameraParameters.hpp"
#include "TextureMap.hpp"
#include "Pipeline.hpp"

namespace tr {
	class Renderer {
	private:
		std::vector<std::unique_ptr<Pipeline>> pipelines;
		std::vector<std::shared_ptr<TextureMap>> textureMaps;
		unsigned int depthBuffer;
		int glutWindow;

		//TODO simplify to one matrix
		mat4f P, VM, ScaleMtx;
		uvec2 resolution;

		void updatePipelines();
		float nearP, farP, FOVy;
		void setGeometry(const Geometry& geometry);

		//Scaling
		bool scaling = true;
		vec3f geoBBxCenter;
		float geoBBxRadius;
		void readConfig(float aspect);
	public:

		enum TextureMapIndex {
			MESH = 0,
			HPOS = 1,
			HDIR = 2,
			LPOS = 3,
			LDIR = 4
		};

		Renderer(const Geometry& geometry, const CameraParameters cam = CameraParameters::default());
		~Renderer();
		inline void setScaling(bool scaling) { this->scaling = scaling; }
		inline mat4f getP() const { return P; }
		void setP(float fovy, float aspect, float nearP, float farP);
		void setP(const mat4f& P);
		vec3f setVM(const mat4f& VM);
		inline mat4f getPVM() const { return P * VM; }
		inline mat4f getVM() const { return VM; }
		inline uvec2 getResolution() { return resolution; }
		void render();
		std::vector<cv::Mat*>getTextures();
	};
}

