#version 450

layout (binding = 1) uniform sampler2D colorMap;
layout (binding = 3) uniform sampler2D testMap;

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec2 inUV;
layout (location = 2) in vec3 inColor;
layout (location = 3) in vec3 inViewVec;
layout (location = 4) in vec3 inLightVec;

layout (location = 0) out vec4 outFragColor;

void main() 
{
	vec3 ambient = vec3(0.0f);

    vec3 lColor = inColor * texture(testMap, inUV).rgb;
	
	// Adjust light calculations for glow color 
	if ((lColor.r >= 0.9) || (lColor.g >= 0.9) || (lColor.b >= 0.9)) 
	{
		ambient = lColor * 0.25;
	}

	vec3 N = normalize(inNormal);
	vec3 L = normalize(inLightVec);
	vec3 V = normalize(inViewVec);
	vec3 R = reflect(-L, N);
	vec3 diffuse = max(dot(N, L), 0.0) * lColor;
	vec3 specular = pow(max(dot(R, V), 0.0), 8.0) * vec3(0.75);
	outFragColor = vec4(ambient + diffuse + specular, 1.0);	
}