#version 460
precision highp float;

layout (location = 0) out vec3 outDirection;

in vec3 direction;

void main() {
	//TODO remove normal compression
	outDirection = direction*0.5+0.5;
}