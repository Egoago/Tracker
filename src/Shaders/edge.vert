#version 460
precision highp float;

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inDirection;

uniform mat4  P, VM;
uniform float near, far;

out vec3 direction;

float getNDCDepth(float cDepth){
	return (-cDepth-near)/(far-near)*2.0-1.0;
}

void main(){
	//uniform depth distribution
	//TODO use linear algebra
	vec4 cPos = VM * vec4(inPosition, 1);
	//offset for z buffer reuse
	cPos.z += 1.5;
	gl_Position = P * cPos;
	gl_Position.z = getNDCDepth(cPos.z)*gl_Position.w;
	direction = inDirection;
}