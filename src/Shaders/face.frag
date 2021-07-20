#version 460
precision highp float;

layout (location = 0) out vec3 outPosition;
layout (location = 1) out vec3 outNormal;

in vec3 position;
in vec3 normal;
//flat in uint index;

void main() {
	outPosition = position;
	outNormal = normal;
	//outPosition = vec3(1.0,1.0,1.0);
	//outIndex = index;
}