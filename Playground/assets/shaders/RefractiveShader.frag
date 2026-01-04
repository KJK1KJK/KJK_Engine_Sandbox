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
	float ratio = 1.00 / 1.52;
	vec3 I = normalize(fragPos);
	vec3 R = refract(I, normalize(normal), ratio);
	R = vec3(inverse(view) * vec4(R, 0.0));
	FragColor = texture(skybox, R);
}