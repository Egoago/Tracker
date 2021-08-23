#pragma once
#include <GL/glew.h>
#include <string>

namespace tr
{
	class Shader
	{
	private:
		GLuint ID, vID, fID;
		std::string vShader, fShader;
		const char* name;
		bool compiled = false;
		void loadShader(GLuint type, std::string filename);
		bool compile();
	public:
		Shader(const char* name);
		~Shader();
		bool enable();
		bool disable();
		void registerFloat4x4(const char* name, const float mtx[]);
		void registerFloat4(const char* name, float f1, float f2, float f3, float f4);
		void registerFloat(const char* name, float value);
	};
}

