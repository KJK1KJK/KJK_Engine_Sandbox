#version 450 core
layout(triangles) in;
layout(line_strip, max_vertices = 6) out;

layout (std140, binding = 0) uniform Matrices
{
	uniform mat4 projection;
	uniform mat4 view;
};

in VS_OUT
{
	vec3 normal;
} gs_in[];

const float MAGNITUDE = 0.03;

void GenerateLine(int index);

void main()
{
	for (int i = 0; i < 3; ++i)
	{
		GenerateLine(i);
	}
}

void GenerateLine(int index)
{
	gl_Position = projection * gl_in[index].gl_Position;
	EmitVertex();

	gl_Position = projection * (gl_in[index].gl_Position + vec4(gs_in[index].normal, 0.0) * MAGNITUDE);
	EmitVertex();

	EndPrimitive();
}