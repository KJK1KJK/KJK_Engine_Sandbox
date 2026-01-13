#version 450 core
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

layout (std140, binding = 0) uniform Matrices
{
	mat4 projection;
	mat4 view;
};

in vec3 vsFragPos[];

uniform float time;

vec4 explode(vec3 position, vec3 normal);
vec3 getNormal();

void main()
{
	vec3 faceNormal = getNormal();

	for(int i = 0; i < 3; i++)
	{
		vec4 explodedPos = explode(vsFragPos[i], faceNormal);
		gl_Position = projection * explodedPos;
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