#version 440

in vec3 bPos;
in vec4 bColor;
layout(location = 4) in vec2 bTexCoord;
layout(location = 5) in vec3 bNormal;
layout(location = 6) in mat4 bInstancedTransform;
layout(location = 10) in mat3 bInstancedNormalTransform;

out vec3 vPos;
out vec4 vSmoothColor;
flat out vec4 vFlatColor;
out vec2 vTexCoord[5];
out vec3 vSmoothNormal;
flat out vec3 vFlatNormal;

uniform mat4 model;
uniform mat4 vp;
uniform mat3 normalMatrix;
uniform int numOfTextures;
uniform mat4 texturesBaseTransform[5];
uniform mat4 texturesCustomTransform[5];
uniform bool sceneCoordTextures;
uniform bool gpuSideInstancedNormalTransforms;

void main()
{
	vec2 texCoord = sceneCoordTextures
		? (bInstancedTransform * model * vec4(bPos, 1.0)).xy
		: bTexCoord;
	for (int i = 0; i < numOfTextures; ++i)
		vTexCoord[i] = vec2(texturesBaseTransform[i] * texturesCustomTransform[i] * vec4(texCoord, 0.0, 1.0)) + vec2(0.5);

	vPos = vec3(bInstancedTransform * model * vec4(bPos, 1.0));

	vSmoothColor = bColor;
	vFlatColor = bColor;

	const vec3 normal = (gpuSideInstancedNormalTransforms ? transpose(inverse(mat3(bInstancedTransform))) : bInstancedNormalTransform) * normalMatrix * bNormal;
	vSmoothNormal = normal;
	vFlatNormal = normal;

	gl_Position = vp * vec4(vPos, 1.0);
}
