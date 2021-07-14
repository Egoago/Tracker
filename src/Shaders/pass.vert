#version 460
precision highp float;

layout (location = 0) in vec3 inPosition;
layout (location = 1) in uint inIndex;

uniform mat4  MVP; // MVP, Model, Model-inverse

out vec3 position;
flat out uint index;

void main(){
	vec4 proj = MVP * vec4(inPosition, 1);
	gl_Position = proj;
	position = proj.xyz;
	index = inIndex;
}