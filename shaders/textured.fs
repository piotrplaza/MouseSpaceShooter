#version 440

in vec2 vTexCoord;

out vec4 fColor;

uniform sampler2D texture1;

void main()
{
	fColor = texture(texture1, vTexCoord);
}
