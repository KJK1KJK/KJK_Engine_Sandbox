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
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constant;
	float linear;
	float quadratic;

	float cutOff;
	float outerCutOff;

	bool isDirectional;
	bool isSpotlight;
};

flat in Light lightView;

void main()
{
	//Ambient light calculations
	vec3 ambient = lightView.ambient * texture(material.diffuse, texCoords).rgb;

	//Emission light calculations
	vec3 emis = texture(material.emission, texCoords).rgb;
	vec3 emission = emis * smoothstep(0.899999999, 0.9, vec3(1.0) - texture(material.specular, texCoords).rgb);

	//Calculate the light direction
	vec3 norm = normalize(normal);
	vec3 lightDir = vec3(0.0);
	if(!lightView.isDirectional) //Light position
	{
		lightDir = normalize(lightView.position - fragPos);
	}
	else //Light direction
	{
		lightDir = normalize(-lightView.direction);
	}
	
	//Spotlight intensity calculations
	float theta = dot(normalize(lightDir), normalize(-lightView.direction));
	float epsilon = lightView.cutOff - lightView.outerCutOff;
	float intesity = clamp((theta - lightView.outerCutOff) / epsilon, 0.0, 1.0);

	if(!lightView.isSpotlight || theta > lightView.outerCutOff)
	{
		//Diffuse light calculations
		float diffuseStrength = max(dot(norm, lightDir), 0.0);
		vec3 diffuse = lightView.diffuse * diffuseStrength * texture(material.diffuse, texCoords).rgb;

		//Specular light calculations
		vec3 viewDir = normalize(-fragPos);
		vec3 reflection = reflect(-lightDir, norm);
		float specularIntensity = pow(max(dot(viewDir, reflection), 0.0), material.shininess);
		vec3 specular = lightView.specular * specularIntensity * texture(material.specular, texCoords).rgb;
	
		//Calculate attenuation
		if(!lightView.isDirectional)
		{
			float distance = length(lightView.position - fragPos);
			float attenuation = 1.0 / (lightView.constant + lightView.linear * distance + lightView.quadratic * pow(distance, 2));
			if (!lightView.isSpotlight)
			{
				ambient *= attenuation;
			}
			diffuse *= attenuation;
			specular *= attenuation;
		}

		//Apply spotlight intensity
		if(lightView.isSpotlight)
		{
			diffuse *= intesity;
			specular *= intesity;
		}

		//Apply all light attributes
		vec3 result = ambient + diffuse + specular + emission;
		FragColor = vec4(result, 1.0);
	}
	else
	{
		FragColor = vec4(ambient + emission, 1.0);
	}
}