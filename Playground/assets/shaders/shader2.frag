#version 450 core

out vec4 FragColor;

uniform float uGreen;

void main()
{
	FragColor = vec4(1.0, uGreen, 1.0, 1.0);
}