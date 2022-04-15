#version 440

in vec3 bPos;
in vec2 bTexCoord;

out vec2 vPos;
out vec2 vTexCoord[5];

uniform mat4 model;
uniform mat4 vp;
uniform int numOfTextures;
uniform mat4 texturesBaseTransform[5];
uniform mat4 texturesCustomTransform[5];

void main()
{
	for (int i = 0; i < numOfTextures; ++i)
		vTexCoord[i] = vec2(texturesCustomTransform[i] * texturesBaseTransform[i] * vec4(bTexCoord, 0.0, 1.0)) + vec2(0.5);
	vPos = (model * vec4(bPos, 1)).xy;

	gl_Position = vp * model * vec4(bPos, 1.0);
}
