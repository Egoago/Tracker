#version 460
precision highp float;

layout (location = 0) out vec3 outPosition;
layout (location = 1) out vec3 outDirection;

in vec3 position;
in vec3 direction;

void main() {
	outPosition = position;
	outDirection = direction;
}