#version 450 core

out vec4 FragColor;

in vec3 normal;
in vec3 fragPos;
in vec3 lightPosView;

uniform vec3 objectColor;
uniform vec3 lightColor;

uniform float ambientStrength;
uniform float specularStrength;

void main()
{
	//Ambient light calculations
	vec3 ambient = ambientStrength * lightColor;

	//Diffuse light calculations
	vec3 norm = normalize(normal);
	vec3 lightDir = normalize(lightPosView - fragPos);
	float diffuseStrength = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diffuseStrength * lightColor;

	//Specular light calculations
	vec3 viewDir = normalize(-fragPos);
	vec3 reflection = reflect(-lightDir, norm);
	float specularIntensity = pow(max(dot(viewDir, reflection), 0.0), 32);
	vec3 specular = specularStrength * specularIntensity * lightColor;

	//Apply all light attributes
	vec3 result = (ambient + diffuse + specular) * objectColor;
	FragColor = vec4(result, 1.0);
}