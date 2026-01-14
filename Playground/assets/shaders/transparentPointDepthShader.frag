#version 450 core
in vec4 FragPos;

uniform vec3 lightPos;
uniform float far_plane;

in vec2 texCoords;
uniform sampler2D texture_diffuse1;

void main()
{
	//Discard transparent fragments
	float alpha = texture(texture_diffuse1, texCoords).a;
	if(alpha < 0.5)
		discard;

	//Get the distance between the fragment and the light source
	float lightDistance = length(FragPos.xyz - lightPos);

	//Map to the [0;1] range by dividing by the far plane
	lightDistance = lightDistance / far_plane;

	//Write this distance as a modified depth
	gl_FragDepth = lightDistance;
}