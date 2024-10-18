#version 440

in vec4 vColor;

out vec4 fColor;

uniform vec4 color;
uniform float forcedAlpha;

void main()
{
	vec4 finalColor = vColor * color;

	if (forcedAlpha >= 0.0)
			finalColor.a = forcedAlpha;

	fColor = finalColor;
}
