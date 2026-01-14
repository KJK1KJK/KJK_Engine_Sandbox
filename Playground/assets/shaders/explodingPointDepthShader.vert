#version 450 core
layout (location = 0) in vec3 aPos;

layout (std140, binding = 0) uniform Matrices
{
	uniform mat4 projection;
	uniform mat4 view;
};

out vec3 vsFragPos;

uniform mat4 model;

void main()
{
	gl_Position = model * vec4(aPos, 1.0);
	vsFragPos = vec3(model * vec4(aPos, 1.0));
}