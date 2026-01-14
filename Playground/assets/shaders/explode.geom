#version 450 core
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

layout (std140, binding = 0) uniform Matrices
{
	mat4 projection;
	mat4 view;
};

struct DirLight
{
	vec3 direction;

	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
};

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
#define NR_POINT_LIGHTS 1

in vec3 vsFragPos[];
in vec3 vsNormal[];
in DirLight vsDirLightView[];
in PointLight vsPointLightsView[][NR_POINT_LIGHTS];
in SpotLight vsSpotLightView[];

out vec3 fragPos;
out vec3 normal;
out DirLight dirLightView;
out PointLight pointLightsView[NR_POINT_LIGHTS];
out SpotLight spotLightView;

in VS_OUT {
	vec2 texCoords;
} gs_in[];

out vec2 texCoords;

in vec3 vsFragPosWorld[];
out vec3 fragPosWorld;

uniform float time;

vec4 explode(vec3 position, vec3 normal);
vec3 getNormal();

uniform mat4 lightSpaceMatrix;
out vec4 fragPosLightSpace;

void main()
{
	vec3 faceNormal = getNormal();

	for(int i = 0; i < 3; i++)
	{
		fragPos = vsFragPos[i];
		normal = vsNormal[i];
		dirLightView = vsDirLightView[i];
		spotLightView = vsSpotLightView[i];
		for(int j = 0; j < NR_POINT_LIGHTS; j++)
		{
			pointLightsView[j] = vsPointLightsView[i][j];
		}

		vec4 explodedPos = explode(vsFragPos[i], faceNormal);
		fragPos = explodedPos.xyz;
		fragPosLightSpace = lightSpaceMatrix * explodedPos;
		gl_Position = projection * explodedPos;
		texCoords = gs_in[i].texCoords;
		fragPosWorld = vsFragPosWorld[i];
		EmitVertex();
	}
	EndPrimitive();
}

vec4 explode(vec3 position, vec3 normal)
{
	float magnitude = 2.0;
	vec3 direction = normal * ((sin(time) + 1.0) / 2.0) * magnitude;
	return vec4(position + direction, 1.0);
}

vec3 getNormal()
{
	vec3 a = vec3(vsFragPos[0]) - vec3(vsFragPos[1]);
	vec3 b = vec3(vsFragPos[2]) - vec3(vsFragPos[1]);
	return normalize(cross(a, b));
}