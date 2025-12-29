#version 450 core

out vec4 FragColor;

in vec3 normal;
in vec3 fragPos;
in vec2 texCoords;

struct Material
{
	sampler2D diffuse;
	sampler2D specular;
	sampler2D emission;
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

struct PointLight
{
	vec3 position;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constant;
	float linear;
	float quadratic;
};
#define NR_POINT_LIGHTS 4
in PointLight pointLightsView[NR_POINT_LIGHTS];
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

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

	//Apply point lighting
	for(int i = 0; i < NR_POINT_LIGHTS; i++)
	{
		result += CalcPointLight(pointLightsView[i], norm, fragPos, viewDir);
	}

	//Apply spotlight lighting
	result += CalcSpotLight(spotLightView, norm, fragPos, viewDir);

	//Calculate emission and apply it
	vec3 emis = texture(material.emission, texCoords).rgb;
	vec3 emission = emis * smoothstep(0.899999999, 0.9, vec3(1.0) - texture(material.specular, texCoords).rgb);
	result += emission;

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
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

	//Combine results
	vec3 ambient = light.ambient * texture(material.diffuse, texCoords).rgb;
	vec3 diffuse = light.diffuse * diff * texture(material.diffuse, texCoords).rgb;
	vec3 specular = light.specular * spec * texture(material.specular, texCoords).rgb;

	//Return final result
	return (ambient + diffuse + specular);
}

//Calculate the point light contribution
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
	//Calculate the light direction
	vec3 lightDir = normalize(light.position - fragPos);

	//Diffuse light calculations
	float diff = max(dot(normal, lightDir), 0.0);

	//Specular light calculations
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

	//Combine results
	vec3 ambient = light.ambient * texture(material.diffuse, texCoords).rgb;
	vec3 diffuse = light.diffuse * diff * texture(material.diffuse, texCoords).rgb;
	vec3 specular = light.specular * spec * texture(material.specular, texCoords).rgb;

	//Calculate attenuation
	float distance = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * pow(distance, 2));

	//Apply attenuation
	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;

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
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

	//Combine results
	vec3 ambient = light.ambient * texture(material.diffuse, texCoords).rgb;
	vec3 diffuse = light.diffuse * diff * texture(material.diffuse, texCoords).rgb;
	vec3 specular = light.specular * spec * texture(material.specular, texCoords).rgb;

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