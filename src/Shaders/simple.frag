#version 460
precision highp float;

out vec4 fragmentColor; // output goes to frame buffer

uniform vec4 color;

in vec3 norm;

void main() {
	fragmentColor = vec4((norm+vec3(1.0))/2.0,1);
}