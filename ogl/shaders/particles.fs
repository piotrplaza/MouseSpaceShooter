#version 440

in vec4 gColor;
in vec2 gTexCoord;

out vec4 fColor;

uniform vec4 color;
uniform sampler2D texture1;

void main()
{
	fColor = texture(texture1, gTexCoord) * gColor * color;
}
