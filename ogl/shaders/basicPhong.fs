#version 440

in vec4 vColor;

out vec4 fColor;

uniform vec4 color;

void main()
{
	fColor = vColor * color;
}
