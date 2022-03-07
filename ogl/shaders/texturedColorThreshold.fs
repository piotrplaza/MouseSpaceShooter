#version 440

in vec2 vTexCoord[5];

out vec4 fColor;

uniform vec4 color;
uniform vec4 mulBlendingColor;
uniform vec4 addBlendingColor;
uniform int numOfTextures;
uniform sampler2D textures[5];
uniform bool alphaFromBlendingTexture;
uniform bool colorAccumulation;
uniform vec3 invisibleColor;
uniform float invisibleColorThreshold;

void main()
{
	const vec4 baseColor = texture(textures[0], vTexCoord[0]);
	const vec4 alternateColor = vec4(vec3(1.0) - vec3(baseColor), 1.0);
	const vec4 finalBaseColor = distance(vec3(baseColor), invisibleColor) < length(vec3(1.0, 1.0, 1.0)) * invisibleColorThreshold
		? alternateColor : baseColor;

	if (numOfTextures == 1)
	{
		fColor = finalBaseColor * color;
	}
	else
	{
		const vec4 finalBlendingColor = (distance(vec3(baseColor), invisibleColor) < length(vec3(1.0, 1.0, 1.0)) * invisibleColorThreshold
			? alternateColor : baseColor) * mulBlendingColor + addBlendingColor;
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
