#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in mat4 instanceMatrix;

out vec2 vsTexCoords;

void main()
{
	gl_Position = instanceMatrix * vec4(aPos, 1.0);
	vsTexCoords = aTexCoords;
}