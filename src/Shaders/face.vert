#version 460
precision highp float;

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
//layout (location = 1) in uint inIndex;

uniform mat4  MVP; // MVP, Model, Model-inverse

out vec3 position;
out vec3 normal;
//flat out uint index;

void main(){
	gl_Position = MVP * vec4(inPosition, 1);
	position = inPosition;
	normal = inNormal*0.49+0.51;
	//index = inIndex;
}