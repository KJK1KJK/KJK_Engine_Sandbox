#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aNormal;

out vec3 ourColor;
out vec2 texCoords;
out vec3 normal;
out vec3 fragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

struct Light {
	vec3 position;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

uniform Light light;

out Light lightView;

void main()
{
	gl_Position = projection * view * model * vec4(aPos, 1.0);
	ourColor = aColor;
	texCoords = aTexCoords;
	normal = mat3(transpose(inverse(view * model))) * aNormal;
	fragPos = vec3(view * model * vec4(aPos, 1.0));

	Light lightV = light;
	lightV.position = vec3(view * vec4(light.position, 1.0));
	lightView = lightV;
}