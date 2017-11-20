#version 430 core

in vec3 newPos;
in vec2 texCoord;

out vec4 outColor;

uniform sampler2D screenTexture;
uniform float time;

void main(void)
{

	vec3 screen = vec3(texture(screenTexture, texCoord));

	// outColor = vec4(0.5 + sin(time)*0.5, 0.8, 0.5, 1.0);
	outColor = vec4(screen, 1.0);
}
