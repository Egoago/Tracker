#pragma once
#include <string>
#include <GL/glew.h>

namespace tr {
	class Shader
	{
	private:
		const GLuint ID;
		const std::string fileName;
		void load();
		void compile();
	public:
		Shader(const std::string& fileName);
		Shader(const std::string& fileName, GLuint type);
		inline GLuint getID() const { return ID; };
		~Shader();
	};
}

