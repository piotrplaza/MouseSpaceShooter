#version 440

in vec3 bPos;
in vec4 bColor;
layout(location = 4) in vec2 bTexCoord;
layout(location = 6) in mat4 bInstancedTransform;

out vec4 vColor;
out vec2 vTexCoord[5];

uniform mat4 model;
uniform mat4 vp;
uniform int numOfTextures;
uniform mat4 texturesBaseTransform[5];
uniform mat4 texturesCustomTransform[5];
uniform bool sceneCoordTextures;

void main()
{
	vec2 texCoord = sceneCoordTextures
		? (model * vec4(bPos, 1.0)).xy
		: bTexCoord;
	for (int i = 0; i < numOfTextures; ++i)
		vTexCoord[i] = vec2(texturesCustomTransform[i] * texturesBaseTransform[i] * vec4(texCoord, 0.0, 1.0)) + vec2(0.5);

	vColor = bColor;
	gl_Position = vp * bInstancedTransform * model * vec4(bPos, 1.0);
}
