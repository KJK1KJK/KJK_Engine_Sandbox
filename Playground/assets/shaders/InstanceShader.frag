#version 450 core

out vec4 FragColor;

in vec3 fragPos;
in vec3 normal;
in vec2 texCoords;

struct Material
{
	sampler2D diffuse;
	sampler2D specular;
	float shininess;
};
uniform Material material;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

struct DirLight
{
	vec3 direction;

	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
};
in DirLight dirLightView;
vec4 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);

struct PointLight
{
	vec3 position;

	vec4 ambient;
	vec4 diffuse;
	vec4 specular;

	float constant;
	float linear;
	float quadratic;
};
#define NR_POINT_LIGHTS 1
in PointLight pointLightsView[NR_POINT_LIGHTS];
vec4 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

struct SpotLight
{
	vec3 position;
	vec3 direction;

	vec4 ambient;
	vec4 diffuse;
	vec4 specular;

	float constant;
	float linear;
	float quadratic;

	float cutOff;
	float outerCutOff;
};
in SpotLight spotLightView;
vec4 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

in vec4 fragPosLightSpace;
uniform sampler2D shadowMap;

vec4 sampleDiffuse();
vec4 sampleSpecular();
float shadowCalculations(vec4 posLightSpace);

void main()
{
	//Calculate the normal and view direction
	vec3 norm = normalize(normal);
	vec3 viewDir = normalize(-fragPos);

	//Apply directional lighting
	vec4 result = CalcDirLight(dirLightView, norm, viewDir);

	//Apply point lighting
	for(int i = 0; i < NR_POINT_LIGHTS; i++)
	{
		result += CalcPointLight(pointLightsView[i], norm, fragPos, viewDir);
	}

	//Apply spotlight lighting
	result += CalcSpotLight(spotLightView, norm, fragPos, viewDir);

	//Set the final fragment color
	FragColor = result;
}

//Calculate the directional light contribution
vec4 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
	//Calculate the light direction
	vec3 lightDir = normalize(-light.direction);

	//Diffuse light calculations
	float diff = max(dot(normal, lightDir), 0.0);

	//Specular light calculations
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);

	//Sample the texture maps uniforms to see which one has been set
	vec4 diffuseTex = sampleDiffuse();
	vec4 specularTex = sampleSpecular();

	//Combine results
	vec4 ambient = light.ambient * diffuseTex;
	vec4 diffuse = light.diffuse * diff * diffuseTex;
	vec4 specular = light.specular * spec * specularTex;

	//Apply shadow
	float shadow = shadowCalculations(fragPosLightSpace);
	diffuse *= (1.0 - shadow);
	specular *= (1.0 - shadow);

	//Return final result
	return (ambient + diffuse + specular);
}

//Calculate the point light contribution
vec4 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
	//Calculate the light direction
	vec3 lightDir = normalize(light.position - fragPos);

	//Diffuse light calculations
	float diff = max(dot(normal, lightDir), 0.0);

	//Specular light calculations
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);

	//Sample the texture maps uniforms to see which one has been set
	vec4 diffuseTex = sampleDiffuse();
	vec4 specularTex = sampleSpecular();

	//Combine results
	vec4 ambient = light.ambient * diffuseTex;
	vec4 diffuse = light.diffuse * diff * diffuseTex;
	vec4 specular = light.specular * spec * specularTex;

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
vec4 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
	//Calculate the light direction
	vec3 lightDir = normalize(light.position - fragPos);

	//Diffuse light calculations
	float diff = max(dot(normal, lightDir), 0.0);

	//Specular light calculations
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);

	//Sample the texture maps uniforms to see which one has been set
	vec4 diffuseTex = sampleDiffuse();
	vec4 specularTex = sampleSpecular();

	//Combine results
	vec4 ambient = light.ambient * diffuseTex;
	vec4 diffuse = light.diffuse * diff * diffuseTex;
	vec4 specular = light.specular * spec * specularTex;

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

vec4 sampleDiffuse()
{
	vec4 m = texture(material.diffuse, texCoords);
	vec4 t = texture(texture_diffuse1, texCoords);

	float mLum = dot(m.rgb, vec3(0.2126, 0.7152, 0.0722));
	return (mLum > 0.001) ? m : t;
}

vec4 sampleSpecular()
{
	vec4 m = texture(material.specular, texCoords);
	vec4 t = texture(texture_specular1, texCoords);

	float mLum = dot(m.rgb, vec3(0.2126, 0.7152, 0.0722));
	return (mLum > 0.001) ? m : t;
}

float shadowCalculations(vec4 posLightSpace)
{
	//Perform perspective divide
	vec3 projCoords = posLightSpace.xyz / posLightSpace.w;
	//Transform the NDC coordinates to [0,1] range
	projCoords = projCoords * 0.5 + 0.5;

	//Check if the fragment is outside the light's frustum
	if(projCoords.z > 1.0)
		return 0.0;

	//Get the closest point on the depth map
	float closestDepth = texture(shadowMap, projCoords.xy).r;
	//Get the current depth from the lights perspective
	float currentDepth = projCoords.z;
	
	//Apply percentage-close filtering
	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
	float bias = max(0.00005 * (1.0 - dot(normalize(normal), normalize(-dirLightView.direction))), 0.000005);
	for(int x = -1; x <= 1; ++x)
	{
		for(int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
		}
	}

	//Return the shadow
	return shadow / 9.0;
}