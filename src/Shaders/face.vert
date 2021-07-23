#version 460
precision highp float;

layout (location = 0) in vec3 inPosition;

uniform mat4  P, VM;
uniform float near, far;

out vec3 position;

float getNDCDepth(float cDepth){
	return (-cDepth-near)/(far-near)*2.0-1.0;
}

void main(){
	//uniform depth distribution
	vec4 cPos = VM * vec4(inPosition, 1);
	gl_Position = P * cPos;
	gl_Position.z = getNDCDepth(cPos.z)*gl_Position.w;
	position = inPosition;
}