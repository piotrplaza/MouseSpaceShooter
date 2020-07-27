#version 440

out vec4 fColor;

uniform vec4 color = vec4(1.0);

void main()
{
	fColor = color;
}
