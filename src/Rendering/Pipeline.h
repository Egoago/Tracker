#pragma once
#include <vector>
#include "TextureMap.h"
#include "Shader.h"

namespace tr
{
	class Pipeline {
		GLuint frameBuffer;
		Shader* shader;
		std::vector<TextureMap*> textureMaps;
		GLbitfield clearMask;
		GLenum drawPrimitive;
		GLuint VAO = 0;
		unsigned int primitiveCount = 0;
		bool drawElements;
	public:
		inline void setGeometry(GLuint vao, unsigned int primitiveCount) {
			VAO = vao;
			this->primitiveCount = primitiveCount;
		}

		Shader* getShader() { return shader; }
		Pipeline(
			const char* name,
			const std::vector<TextureMap*>& textureMaps,
			GLenum drawPrimitive,
			GLbitfield clearMask,
			GLuint depthBuffer,
			bool drawElements);
		~Pipeline();
		void render(std::vector<cv::Mat*>& outTextures);
	};
}

