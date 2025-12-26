#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 ourColor;
out vec4 vertexPosition;

uniform float uHorizontalOffset;

void main()
{
	gl_Position = vec4(aPos, 1.0) * vec4(1.0, -1.0, 1.0, 1.0) + vec4(uHorizontalOffset, 0.0, 0.0, 0.0);
	ourColor = aColor;
	vertexPosition = gl_Position;
}