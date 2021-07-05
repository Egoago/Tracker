#pragma once
#include <GL/glew.h>
#include <string>

class Shader
{
	GLuint ID, vID, fID;
	std::string vShader, fShader;
	bool compiled = false;
public:
	void loadShader(GLuint type, std::string filename);
	bool compile();
	bool enable();
	bool disable();
	void registerMVP(GLfloat* m);
};

