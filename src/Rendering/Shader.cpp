#include "Shader.hpp"
#include <fstream>
#include <vector>
#include <iostream>
#include "../Misc/Links.hpp"
#include "../Misc/Log.hpp"

using namespace tr;

std::string readFile(std::string filename) {
	std::ifstream stream(filename);
	if (!stream.is_open()) {
		std::cerr << "File " << filename << " not found.\n";
		exit(1);
	}
	std::string str((std::istreambuf_iterator<char>(stream)),
		(std::istreambuf_iterator<char>()));
	return str;
}

Shader::Shader(const char* name) : name(name) {
	loadShader(GL_VERTEX_SHADER, SHADERS_FOLDER + std::string(name) + ".vert");
	loadShader(GL_FRAGMENT_SHADER, SHADERS_FOLDER + std::string(name) + ".frag");
	compile();
}

void Shader::loadShader(GLuint type, std::string filename) {
	switch (type)
	{
	case GL_VERTEX_SHADER:
		vID = glCreateShader(GL_VERTEX_SHADER);
		vShader = readFile(filename);
		break;

	case GL_FRAGMENT_SHADER:
		fID = glCreateShader(GL_FRAGMENT_SHADER);
		fShader = readFile(filename);
		break;
	default: {
		std::cerr << "Not implemented shader type: " << type << std::endl;
		exit(1);
	}
	}
}

bool Shader::compile() {
	GLint result = GL_FALSE;
	int InfoLogLength;

	//Vertex shader
	const char* vertexShaderSourcePtr = vShader.c_str();
	glShaderSource(vID, 1, &vertexShaderSourcePtr, NULL);
	glCompileShader(vID);

	glGetShaderiv(vID, GL_COMPILE_STATUS, &result);
	if (result != GL_TRUE)
	{
		glGetShaderiv(vID, GL_INFO_LOG_LENGTH, &InfoLogLength);
		std::vector<char> vertexShaderErrorMessage(InfoLogLength);
		glGetShaderInfoLog(vID, InfoLogLength, NULL, &vertexShaderErrorMessage[0]);
		std::cerr << "Vertex shader: " << vertexShaderErrorMessage.data() << std::endl;
	}

	//Fragment shader
	const char* fragmentShaderSourcePtr = fShader.c_str();
	glShaderSource(fID, 1, &fragmentShaderSourcePtr, NULL);
	glCompileShader(fID);

	glGetShaderiv(fID, GL_COMPILE_STATUS, &result);
	if (result != GL_TRUE)
	{
		glGetShaderiv(fID, GL_INFO_LOG_LENGTH, &InfoLogLength);
		std::vector<char> fragmentShaderErrorMessage(InfoLogLength);
		glGetShaderInfoLog(fID, InfoLogLength, NULL, &fragmentShaderErrorMessage[0]);
		std::cerr << "Fragment shader: " << fragmentShaderErrorMessage.data() << std::endl;
	}
	
	ID = glCreateProgram();
	glAttachShader(ID, vID);
	glAttachShader(ID, fID);

	glBindAttribLocation(ID, 0, "position");
	glBindAttribLocation(ID, 1, "normal");

	glLinkProgram(ID);
	glGetProgramiv(ID, GL_LINK_STATUS, &result);
	if (result != GL_TRUE)
	{
		glGetProgramiv(ID, GL_INFO_LOG_LENGTH, &InfoLogLength);
		std::vector<char> programErrorMessage(InfoLogLength);
		glGetProgramInfoLog(ID, InfoLogLength, NULL, &programErrorMessage[0]);
		std::cerr << "Link: " << programErrorMessage.data() << std::endl;
	}
	else compiled = true;

	glDeleteShader(vID);
	glDeleteShader(fID);

	return true;
}

bool Shader::enable()
{
    if (compiled)
        glUseProgram(ID);
    return compiled;
}

bool Shader::disable()
{
    glUseProgram(0);
    return compiled;
}

Shader::~Shader() {
	glDeleteProgram(ID);
}

void Shader::registerFloat4x4(const char* name, float mtx[]) {
	GLuint id = glGetUniformLocation(ID, name);
	if (id == -1) {
		Logger::warning(
			"uniform mat4 variable"
			+ std::string(name)
			+ " not found in shader "
			+ std::string(this->name));
		return;
	}
	glUniformMatrix4fv(id, 1, GL_FALSE, mtx);
}

void Shader::registerFloat4(const char* name, float f1, float f2, float f3, float f4) {
	GLuint id = glGetUniformLocation(ID, name);
	if (id == -1) {
		Logger::warning(
			"uniform vec4 variable"
			+ std::string(name)
			+ " not found in shader "
			+ std::string(this->name));
		return;
	}
	glUniform4f(id, f1, f2, f3, f4);
}

void Shader::registerFloat(const char* name, float value) {
	GLuint id = glGetUniformLocation(ID, name);
	if (id == -1) {
		Logger::warning(
			"uniform float variable"
			+ std::string(name)
			+ " not found in shader "
			+ std::string(this->name));
		return;
	}
	glUniform1f(id, value);
}
