#include "Shader.h"
#include <fstream>
#include <vector>
#include <iostream>

std::string readFile(std::string filename)
{
	std::ifstream stream(filename, std::ios::in);
	std::string str((std::istreambuf_iterator<char>(stream)),
		(std::istreambuf_iterator<char>()));
	return str;
}

void Shader::loadShader(GLuint type, std::string filename)
{
	switch (type)
	{
	case GL_VERTEX_SHADER:
		vID = glCreateShader(GL_VERTEX_SHADER);
		vShader = readFile(filename);
		break;

	case GL_FRAGMENT_SHADER:
		fID = glCreateShader(GL_VERTEX_SHADER);
		fShader = readFile(filename);
		break;
	default:
		throw "Not implemented shader type: " + type;
	}
}

bool Shader::compile()
{
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
		std::cout << "Fragment shader: " << fragmentShaderErrorMessage.data() << std::endl;
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
		std::cout << "Link: " << programErrorMessage.data() << std::endl;
	}

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


void Shader::registerMVP(GLfloat* m)
{
	GLuint id = glGetUniformLocation(ID, "MVP");
	glUniformMatrix4fv(id, 1, GL_FALSE, m);
}