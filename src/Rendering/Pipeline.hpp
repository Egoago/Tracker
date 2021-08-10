#pragma once
#include <vector>
#include <memory>
#include "TextureMap.hpp"
#include "Shader.hpp"
#include "../Misc/Base.hpp"

namespace tr {
	class Pipeline {
		GLuint frameBuffer;
		std::shared_ptr<Shader> shader;
		const std::vector<std::shared_ptr<TextureMap>>& textureMaps;
		GLbitfield clearMask;
		GLenum drawPrimitive;
		GLuint VAO = 0;

		uint primitiveCount = 0;
		bool drawElements;
	public:
		inline void setGeometry(GLuint vao, uint primitiveCount) {
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

