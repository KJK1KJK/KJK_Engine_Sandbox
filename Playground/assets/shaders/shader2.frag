#version 450 core

out vec4 FragColor;

uniform float uGreen;

void main()
{
	FragColor = vec4(0.0, uGreen, 0.0, 1.0);
}