#version 440

in vec3 bPos;
in vec2 bTexCoord;

out vec2 vTexCoord;

uniform mat4 model;
uniform mat4 vp;
uniform int numOfTextures;
uniform vec2 texturesTranslate[5];
uniform vec2 texturesScale[5];

void main()
{
	vTexCoord = bTexCoord / texturesScale[0] - texturesTranslate[0];
	gl_Position = vp * model * vec4(bPos, 1.0);
}
