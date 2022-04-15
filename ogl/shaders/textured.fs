#version 440

in vec2 vPos;
in vec2 vTexCoord[5];

out vec4 fColor;

uniform vec4 color;
uniform vec4 mulBlendingColor;
uniform vec4 addBlendingColor;
uniform int numOfTextures;
uniform sampler2D textures[5];
uniform bool alphaFromBlendingTexture;
uniform bool colorAccumulation;
uniform int numOfPlayers;
uniform vec2 playersCenter[4];
uniform float playerUnhidingRadius;

float playersDistanceAlpha()
{
	if (playerUnhidingRadius == 0.0)
		return 1.0;

	float minDist = 1000000;
	for (int i = 0; i < numOfPlayers; ++i)
		minDist = min(minDist, distance(vPos, playersCenter[i]));

	if (minDist < playerUnhidingRadius)
		return minDist / playerUnhidingRadius;

	return 1.0;
}

void main()
{
	if (numOfTextures == 1)
		fColor = texture(textures[0], vTexCoord[0]) * color * playersDistanceAlpha();
	else
	{
		const vec4 finalBlendingColor = texture(textures[0], vTexCoord[0]) * mulBlendingColor + addBlendingColor;
		vec4 accumulatedColor = vec4(0.0);

		for (int i = 1; i < numOfTextures; ++i)
			accumulatedColor += finalBlendingColor[i - 1] * texture(textures[i], vTexCoord[i]);

		fColor = vec4((accumulatedColor / (colorAccumulation ? 1 : (numOfTextures - 1))).rgb,
			alphaFromBlendingTexture ? finalBlendingColor.a : accumulatedColor.a
		) * color * playersDistanceAlpha();
	}
}
