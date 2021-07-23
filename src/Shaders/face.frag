#version 460
precision highp float;

layout (location = 0) out vec3 outPosition;

in vec3 position;
//flat in uint index;

//out vec4 FragColor;
//
//float near = 0.1; 
//float far  = 100.0; 

//float LinearizeDepth(float depth) 
//{
//    float z = depth * 2.0 - 1.0; // back to NDC 
//    return (2.0 * near * far) / (far + near - z * (far - near));	
//}

void main() {
	outPosition = position;
	//outPosition = vec3(1.0,1.0,1.0);
	//outIndex = index;            
//    float depth = LinearizeDepth(gl_FragCoord.z) / far; // divide by far for demonstration
//    FragColor = vec4(vec3(depth), 1.0);
}

  