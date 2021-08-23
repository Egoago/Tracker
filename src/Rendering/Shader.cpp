#include "Shader.hpp"
#include <fstream>
#include <vector>
#include <unordered_map>
#include "../Misc/Links.hpp"
#include "../Misc/Log.hpp"

using namespace tr;

GLuint getType(const std::string& fileName) {
	static const std::unordered_map<std::string, GLuint> shaderTypes = {
		{std::string(VERTEX_SHADER_EXTENSION),   GL_VERTEX_SHADER},
		{std::string(FRAGMENT_SHADER_EXTENSION), GL_FRAGMENT_SHADER}
	};
	const std::string fileExtension = fileName.substr(fileName.find('.'));
	if (shaderTypes.count(fileExtension) == 0)
		Logger::error("Unsupported shader type");
	return shaderTypes.at(fileExtension);
}

Shader::Shader(const std::string& fileName) :
	Shader(fileName, getType(fileName)) {}

Shader::Shader(const std::string& fileName, GLuint type) :
	fileName(fileName),
	ID(glCreateShader(type)) {
	load();
	compile();
}

void Shader::load() {
	std::ifstream stream(SHADERS_FOLDER + fileName);
	if (!stream.is_open())
		Logger::error("Shader source not found");
	std::string str((std::istreambuf_iterator<char>(stream)),
					(std::istreambuf_iterator<char>()));
	const char* shaderSource = str.c_str();
	glShaderSource(ID, 1, &shaderSource, NULL);
}

void Shader::compile() {
	GLint result = GL_FALSE;
	int InfoLogLength;

	glCompileShader(ID);

	glGetShaderiv(ID, GL_COMPILE_STATUS, &result);
	if (result != GL_TRUE)
	{
		glGetShaderiv(ID, GL_INFO_LOG_LENGTH, &InfoLogLength);
		std::vector<char> shaderErrorMessage(InfoLogLength);
		glGetShaderInfoLog(ID, InfoLogLength, NULL, &shaderErrorMessage[0]);
		Logger::error(shaderErrorMessage.data());
	}
}

Shader::~Shader() {
	glDeleteShader(ID);
}
