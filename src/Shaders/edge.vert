#version 460
precision highp float;

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inDirection;

uniform mat4  MVP;

out vec3 direction;

void main(){
	gl_Position = MVP * vec4(inPosition, 1);
	//offset for z buffer reuse
	gl_Position.z -= 0.01;
	direction = inDirection;
	//index = inIndex;
}