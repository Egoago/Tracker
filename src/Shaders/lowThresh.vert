#version 460
precision highp float;

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inDirection;

uniform mat4  P, VM;
uniform float near, far;

//TODO add to config
// it's already in obj.conf
const float rasterOffset = 0.1; //in mms

out vec3 pos;
out vec3 offsetPos;

float getNDCDepth(float cDepth){
	return (-cDepth-near)/(far-near)*2.0-1.0;
}

void main(){
	//uniform depth distribution
	//TODO use linear algebra
	vec4 cPos = VM * vec4(inPosition, 1);
	gl_Position = P * cPos;
	gl_Position.z = getNDCDepth(cPos.z)*gl_Position.w;
	pos = inPosition;
	offsetPos = inPosition + inDirection * rasterOffset; //dir normalized in assimp
}