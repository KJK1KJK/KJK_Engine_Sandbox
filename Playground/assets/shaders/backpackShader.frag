#version 450 core

out vec4 FragColor;

in vec3 normal;
in vec3 fragPos;
in vec2 texCoords;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

struct Material
{
	float shininess;
};
uniform Material material;

struct DirLight
{
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
in DirLight dirLightView;
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);

struct SpotLight
{
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
in SpotLight spotLightView;
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
	//Calculate the normal and view direction
	vec3 norm = normalize(normal);
	vec3 viewDir = normalize(-fragPos);

	//Apply directional lighting
	vec3 result = CalcDirLight(dirLightView, norm, viewDir);

	//Apply spotlight lighting
	result += CalcSpotLight(spotLightView, norm, fragPos, viewDir);

	//Set the final fragment color
	FragColor = vec4(result, 1.0);
}

//Calculate the directional light contribution
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
	//Calculate the light direction
	vec3 lightDir = normalize(-light.direction);

	//Diffuse light calculations
	float diff = max(dot(normal, lightDir), 0.0);

	//Specular light calculations
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);

	//Combine results
	vec3 ambient = light.ambient * texture(texture_diffuse1, texCoords).rgb;
	vec3 diffuse = light.diffuse * diff * texture(texture_diffuse1, texCoords).rgb;
	vec3 specular = light.specular * spec * texture(texture_specular1, texCoords).rgb;

	//Return final result
	return (ambient + diffuse + specular);
}

//Calculate the spotlight contribution
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
	//Calculate the light direction
	vec3 lightDir = normalize(light.position - fragPos);

	//Diffuse light calculations
	float diff = max(dot(normal, lightDir), 0.0);

	//Specular light calculations
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);

	//Combine results
	vec3 ambient = light.ambient * texture(texture_diffuse1, texCoords).rgb;
	vec3 diffuse = light.diffuse * diff * texture(texture_diffuse1, texCoords).rgb;
	vec3 specular = light.specular * spec * texture(texture_specular1, texCoords).rgb;

	//Calculate attenuation
	float distance = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * pow(distance, 2));

	//Apply attenuation
	diffuse *= attenuation;
	specular *= attenuation;

	//Spotlight intensity calculations
	float theta = dot(lightDir, normalize(-light.direction));
	float epsilon = light.cutOff - light.outerCutOff;
	float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

	//Apply spotlight intensity
	ambient *= intensity;
	diffuse *= intensity;
	specular *= intensity;

	//Return final result
	return (ambient + diffuse + specular);
}