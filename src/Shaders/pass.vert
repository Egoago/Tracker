#version 460
precision highp float;

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;

uniform mat4  MVP; // MVP, Model, Model-inverse

void main(){
	gl_Position = vec4(pos, 1) * MVP;
}