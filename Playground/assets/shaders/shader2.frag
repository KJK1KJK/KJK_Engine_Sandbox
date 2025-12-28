#version 450 core

out vec4 FragColor;

in vec3 normal;
in vec3 fragPos;
in vec2 texCoords;

struct Material {
	sampler2D diffuse;
	sampler2D specular;
	sampler2D emission;
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
	vec3 ambient = lightView.ambient * texture(material.diffuse, texCoords).rgb;

	//Diffuse light calculations
	vec3 norm = normalize(normal);
	vec3 lightDir = normalize(lightView.position - fragPos);
	float diffuseStrength = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = lightView.diffuse * diffuseStrength * texture(material.diffuse, texCoords).rgb;

	//Specular light calculations
	vec3 viewDir = normalize(-fragPos);
	vec3 reflection = reflect(-lightDir, norm);
	float specularIntensity = pow(max(dot(viewDir, reflection), 0.0), material.shininess);
	vec3 specular = lightView.specular * specularIntensity * texture(material.specular, texCoords).rgb;

	//Emission light calculations
	vec3 emis = texture(material.emission, texCoords).rgb;
	vec3 emission = emis * smoothstep(0.899999999, 0.9, vec3(1.0) - texture(material.specular, texCoords).rgb);

	//Apply all light attributes
	vec3 result = ambient + diffuse + specular + emission;
	FragColor = vec4(result, 1.0);
}