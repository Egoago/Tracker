#pragma once
#include <vector>
#include <memory>
#include <GL/glew.h>
#include "TextureMap.hpp"
#include "Shader.hpp"
#include "../Misc/Base.hpp"

namespace tr {
	class Pipeline {
		const GLuint ID;
		GLuint frameBuffer;
		const std::vector<std::shared_ptr<TextureMap>>& textureMaps;
		GLbitfield clearMask;
		GLenum drawPrimitive;
		GLuint VAO = 0;

		uint primitiveCount = 0;
		bool drawElements;
		void link();
		void bind(const std::vector<Shader*>& shaders,
				  GLuint depthBuffer);
	public:
		inline void setGeometry(GLuint vao, uint primitiveCount) {
			VAO = vao;
			this->primitiveCount = primitiveCount;
		}

		Pipeline(
			const std::vector<std::shared_ptr<TextureMap>>& textureMaps,
			const std::vector<Shader*>& shaders,
			GLenum drawPrimitive,
			GLbitfield clearMask,
			GLuint depthBuffer,
			bool drawElements);


		void uniform(const char* name, const vec4f& data);
		void uniform(const char* name, const mat4f& data);
		void uniform(const char* name, const float value);
		~Pipeline();
		void render();
	};
}

