#version 450 core
out vec4 FragColor;

in vec2 texCoords;

uniform sampler2D screenTexture;

void main()
{
	//Get the texture color
	vec4 results = texture(screenTexture, texCoords);

	//Apply gamma correction
	results.rgb = pow(results.rgb, vec3(1.0/2.2));

	FragColor = results;
}