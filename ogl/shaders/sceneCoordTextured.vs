#version 440

in vec3 bPos;

out vec2 vTexCoord;

uniform mat4 model;
uniform mat4 vp;
uniform int numOfTextures;
uniform vec2 texturesTranslate[5];
uniform vec2 texturesScale[5];
uniform bool textureCoordBasedOnModelTransform;

void main()
{
	vTexCoord = ((textureCoordBasedOnModelTransform ? model : mat4(1.0)) * vec4(bPos, 1.0)).xy / texturesScale[0] - texturesTranslate[0];
	gl_Position = vp * model * vec4(bPos, 1.0);
}
