#version 450 core
out vec4 FragColor;

in vec2 texCoords;

uniform sampler2D depthMap;

void main()
{
	float depthValue = texture(depthMap, texCoords).r;

	depthValue = pow(1.0 - depthValue, 4.0);

	FragColor = vec4(vec3(depthValue), 1.0);
}