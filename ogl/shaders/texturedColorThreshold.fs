#version 440

in vec2 vTexCoord;

out vec4 fColor;

uniform vec4 color = vec4(1.0);
uniform sampler2D texture1;
uniform vec3 invisibleColor = vec3(0.0);
uniform float invisibleColorThreshold = 0.0;

void main()
{
	vec4 baseColor = texture(texture1, vTexCoord) * color;
	fColor = distance(vec3(baseColor), invisibleColor) < invisibleColorThreshold ? vec4(0.0) : baseColor;
}
