#version 440

in vec2 vTexCoord;

out vec4 fColor;

uniform vec4 color;
uniform int numOfTextures;
uniform sampler2D textures[5];
uniform vec3 invisibleColor;
uniform float invisibleColorThreshold;

void main()
{
	vec4 baseColor = texture(textures[0], vTexCoord) * color;
	vec4 alternateColor = vec4(vec3(1.0) - vec3(baseColor), 1.0);
	fColor = distance(vec3(baseColor), invisibleColor) < length(vec3(1.0, 1.0, 1.0)) * invisibleColorThreshold
	? alternateColor
	: baseColor;
}
