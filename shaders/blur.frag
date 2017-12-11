#version 430 core


in vec2 TexCoords;
out vec4 color;

uniform int blurRadius;
uniform int enableBlur;
uniform float nSamples;

uniform sampler2D screenTexture;

void main()
{
	vec4 colorVal =  texture(screenTexture, TexCoords);

	if(enableBlur == 0)
	{
		color = colorVal;
		return;
	}

	// Set the blur radius and increment depending on values given from user.
	float radius = blurRadius;
	float increment = radius / nSamples;

	// Initialize variables.
	vec4 finalColor = vec4(0.0f);
	int totalSamples = 0;
	
	// Perform some simple mean-blur, box-shaped.
	for(float i = 0; i < radius * 2 + 1; i += increment)
	{
		for(float j = 0; j < radius * 2 + 1; j += increment)
		{
			vec4 samplePoint = texture(screenTexture, TexCoords + (1.0f / 1000.0f) * vec2(i - (radius * 2 + 1) / 2.0f, j - (radius * 2 + 1) / 2.0f));
			if(samplePoint.x < 0 || samplePoint.y < 0 || samplePoint.z < 0 || samplePoint.w < 0)
				continue;
			finalColor += samplePoint;
			totalSamples++;
		}
	}


	finalColor = finalColor / totalSamples;

	color = finalColor;
}