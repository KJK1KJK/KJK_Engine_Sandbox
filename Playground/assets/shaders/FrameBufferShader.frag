#version 450 core
out vec4 FragColor;

in vec2 texCoords;

uniform sampler2D screenTexture;

const float offeset = 1.0 / 300.0;

void main()
{
	vec2 offsets[9] = vec2[](
		vec2(-offeset,  offeset), //top-left
		vec2( 0.0f,    offeset),  //top-center
		vec2( offeset,  offeset), //top-right
		vec2(-offeset,  0.0f),    //center-left
		vec2( 0.0f,    0.0f),     //center-center
		vec2( offeset,  0.0f),    //center-right
		vec2(-offeset, -offeset), //bottom-left
		vec2( 0.0f,   -offeset),  //bottom-center
		vec2( offeset, -offeset)  //bottom-right
	);

	float kernel[9] = float[](
		 1.0,  2.0,  1.0,
		 2.0,  4.0,  2.0,
		 1.0,  2.0,  1.0
	);
	for(int i = 0; i < 9; i++)
	{
		kernel[i] /= 16.0;
	}

	vec3 sampleTex[9];
	for(int i = 0; i < 9; i++)
	{
		sampleTex[i] = vec3(texture(screenTexture, texCoords.st + offsets[i]));
	}
	vec3 col = vec3(0.0);
	for(int i = 0; i < 9; i++)
	{
		col += sampleTex[i] * kernel[i];
	}

	FragColor = vec4(col, 1.0);
}