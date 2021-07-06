#version 460
precision highp float;

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;

uniform mat4  MVP; // MVP, Model, Model-inverse

out vec3 norm;

void main(){
	gl_Position = MVP * vec4(pos, 1);
	norm = normal;
}