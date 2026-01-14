#version 450 core
in vec4 FragPos;

uniform vec3 lightPos;
uniform float far_plane;

void main()
{
	//Get the distance between the fragment and the light source
	float lightDistance = length(FragPos.xyz - lightPos);

	//Map to the [0;1] range by dividing by the far plane
	lightDistance = lightDistance / far_plane;

	//Write this distance as a modified depth
	gl_FragDepth = lightDistance;
}