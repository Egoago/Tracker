#version 460
precision highp float;

layout (location = 0) out vec3 outPosition;
layout (location = 1) out uint outIndex;

in vec3 position;
flat in uint index;

void main() {
	outPosition = position;
	outIndex = index;
}