#version 460
precision highp float;

layout (location = 0) out vec3 outPosition;
layout (location = 1) out vec3 outDirection;

layout(binding=0) uniform sampler2D silhouette;

in vec3 position;
in vec3 direction;

void main() {
    // if the pixel is black (we are on the silhouette)
    const vec2 coord = gl_FragCoord.xy;
    const vec2 size = 1.0 / textureSize(silhouette, 0);
    const ivec2 oneZero = ivec2(1,0);
    const ivec2 zeroOne = ivec2(0,1);
    if (texture(silhouette, coord).xyz != vec3(1.0) ||
        (texture(silhouette, coord + size.xy * zeroOne).xyz != vec3(0.0) &&
         texture(silhouette, coord - size.xy * zeroOne).xyz != vec3(0.0) &&
         texture(silhouette, coord + size.xy * oneZero).xyz != vec3(0.0) &&
         texture(silhouette, coord - size.xy * oneZero).xyz != vec3(0.0)))
        discard;
    outPosition = position;
	outDirection = direction;
}