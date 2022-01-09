#version 460
precision highp float;

layout (location = 0) in vec3 inPosition;

uniform mat4  P, VM;
uniform float near, far;

float getNDCDepth(float cDepth){
	return (-cDepth-near)/(far-near)*2.0-1.0;
}

void main(){
	//uniform depth distribution
	vec4 cPos = VM * vec4(inPosition, 1);
	gl_Position = P * cPos;
	//offset against z fighting in z buffer reuse
	cPos.z -= 1.5;
	gl_Position.z = getNDCDepth(cPos.z)*gl_Position.w;
}