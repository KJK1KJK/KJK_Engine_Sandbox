#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 texCoords;
out vec3 normal;
out vec3 fragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

struct DirLight {
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
uniform DirLight dirLight;
out DirLight dirLightView;

struct PointLight {
	vec3 position;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constant;
	float linear;
	float quadratic;
};
#define NR_POINT_LIGHTS 4
uniform PointLight pointLights[NR_POINT_LIGHTS];
out PointLight pointLightsView[NR_POINT_LIGHTS];

struct SpotLight {
	vec3 position;
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constant;
	float linear;
	float quadratic;

	float cutOff;
	float outerCutOff;
};
uniform SpotLight spotLight;
out SpotLight spotLightView;

void main()
{
	gl_Position = projection * view * model * vec4(aPos, 1.0);
	texCoords = aTexCoords;
	normal = mat3(transpose(inverse(view * model))) * aNormal;
	fragPos = vec3(view * model * vec4(aPos, 1.0));

	DirLight dirLightV = dirLight;
	dirLightV.direction = vec3(view * vec4(dirLight.direction, 0.0));
	dirLightView = dirLightV;

	for(int i = 0; i < NR_POINT_LIGHTS; i++)
	{
		PointLight pointLightV = pointLights[i];
		pointLightV.position = vec3(view * vec4(pointLights[i].position, 1.0));
		pointLightsView[i] = pointLightV;
	}

	SpotLight spotLightV = spotLight;
	spotLightV.position = vec3(view * vec4(spotLight.position, 1.0));
	spotLightV.direction = vec3(view * vec4(spotLight.direction, 0.0));
	spotLightView = spotLightV;
}