#version 450 core

in vec2 texCoords;
uniform sampler2D texture_diffuse1;

void main()
{
	float alpha = texture(texture_diffuse1, texCoords).a;
	if(alpha < 0.5)
		discard;
}