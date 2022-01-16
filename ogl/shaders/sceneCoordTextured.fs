#version 440

in vec2 vTexCoord[5];

out vec4 fColor;

uniform vec4 color;
uniform vec4 blendingColor;
uniform int numOfTextures;
uniform sampler2D textures[5];
uniform bool alphaFromBlendingTexture;
uniform bool colorAccumulation;

void main()
{
	if (numOfTextures == 1)
		fColor = texture(textures[0], vTexCoord[0]) * color;
	else
	{
		const vec4 finalBlendingColor = texture(textures[0], vTexCoord[0]) * blendingColor;
		vec4 accumulatedColor = vec4(0.0);
		for (int i = 1; i < numOfTextures; ++i)
			accumulatedColor += finalBlendingColor[i - 1] * texture(textures[i], vTexCoord[i]);
		fColor = vec4((accumulatedColor / (colorAccumulation ? 1 : (numOfTextures - 1))).rgb,
			alphaFromBlendingTexture
			? finalBlendingColor.a
			: accumulatedColor.a
		) * color;
	}
}
