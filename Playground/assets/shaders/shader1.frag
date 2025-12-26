#version 450 core

out vec4 FragColor;

in vec3 ourColor;
in vec4 vertexPosition;

void main()
{
	FragColor = vertexPosition;
}