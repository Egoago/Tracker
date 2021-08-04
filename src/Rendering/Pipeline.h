#pragma once
#include <vector>
#include <memory>
#include "TextureMap.h"
#include "Shader.h"

namespace tr
{
	class Pipeline {
		GLuint frameBuffer;
		std::shared_ptr<Shader> shader;
		const std::vector<std::shared_ptr<TextureMap>>& textureMaps;
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

		std::shared_ptr<Shader> getShader() { return shader; }
		Pipeline(
			const char* name,
			const std::vector<std::shared_ptr<TextureMap>>& textureMaps,
			GLenum drawPrimitive,
			GLbitfield clearMask,
			GLuint depthBuffer,
			bool drawElements);
		~Pipeline();
		void render();
	};
}

