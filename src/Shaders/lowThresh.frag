#version 460
precision highp float;

layout (location = 0) out vec3 outPos;
layout (location = 1) out vec3 outOffsetPos;

in vec3 pos;
in vec3 offsetPos;

void main() {
	outPos = pos;
	outOffsetPos = offsetPos;
}