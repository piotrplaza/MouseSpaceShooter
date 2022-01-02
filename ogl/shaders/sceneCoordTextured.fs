#version 440

in vec2 vTexCoord;

out vec4 fColor;

uniform vec4 color = vec4(1.0);
uniform int numOfTextures = 1;
uniform sampler2D textures[5];

void main()
{
	fColor = texture(textures[0], vTexCoord) * color;
}
