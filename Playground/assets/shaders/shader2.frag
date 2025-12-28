#version 450 core

out vec4 FragColor;

in vec3 normal;
in vec3 fragPos;

struct Material {
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
};

uniform Material material;

struct Light {
	vec3 position;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

in Light lightView;

void main()
{
	//Ambient light calculations
	vec3 ambient = lightView.ambient * material.ambient;

	//Diffuse light calculations
	vec3 norm = normalize(normal);
	vec3 lightDir = normalize(lightView.position - fragPos);
	float diffuseStrength = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = lightView.diffuse * (diffuseStrength * material.diffuse);

	//Specular light calculations
	vec3 viewDir = normalize(-fragPos);
	vec3 reflection = reflect(-lightDir, norm);
	float specularIntensity = pow(max(dot(viewDir, reflection), 0.0), material.shininess);
	vec3 specular = lightView.specular * (specularIntensity * material.specular);

	//Apply all light attributes
	vec3 result = ambient + diffuse + specular;
	FragColor = vec4(result, 1.0);
}