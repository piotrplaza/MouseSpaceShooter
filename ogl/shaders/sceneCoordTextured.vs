#version 440

in vec3 bPos;

out vec2 vTexCoord[5];

uniform mat4 model;
uniform mat4 vp;
uniform int numOfTextures;
uniform mat4 texturesBaseTransform[5];
uniform bool textureCoordBasedOnModelTransform;
uniform mat4 texturesCustomTransform[5];

void main()
{
	for (int i = 0; i < numOfTextures; ++i)
		vTexCoord[i] = vec2(texturesCustomTransform[i] * texturesBaseTransform[i] * ((textureCoordBasedOnModelTransform ? model : mat4(1.0)) * vec4(bPos, 1.0)));

	gl_Position = vp * model * vec4(bPos, 1.0);
}
