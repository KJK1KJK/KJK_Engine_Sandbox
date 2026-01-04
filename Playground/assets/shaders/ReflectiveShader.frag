#version 450 core
out vec4 FragColor;

in vec3 normal;
in vec3 fragPos;

uniform vec3 cameraPos;
uniform samplerCube skybox;

layout (std140, binding = 0) uniform Matrices
{
	uniform mat4 projection;
	uniform mat4 view;
};
void main()
{
	vec3 I = normalize(fragPos);
	vec3 R = reflect(I, normalize(normal));
	R = vec3(inverse(view) * vec4(R, 0.0));
	FragColor = texture(skybox, R);
}