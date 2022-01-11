#version 440

in vec2 vTexCoord[5];

out vec4 fColor;

uniform vec4 color;
uniform int numOfTextures;
uniform sampler2D textures[5];
uniform vec3 invisibleColor;
uniform float invisibleColorThreshold;

void main()
{
	const vec4 baseColor = texture(textures[0], vTexCoord[0]);
	const vec4 alternateColor = vec4(vec3(1.0) - vec3(baseColor), 1.0);
	const vec4 finalBaseColor = distance(vec3(baseColor), invisibleColor) < length(vec3(1.0, 1.0, 1.0)) * invisibleColorThreshold
			? alternateColor
			: baseColor;

	if (numOfTextures == 1)
	{
		fColor = finalBaseColor * color;
	}
	else
	{
		const vec4 blendingColor = distance(vec3(baseColor), invisibleColor) < length(vec3(1.0, 1.0, 1.0)) * invisibleColorThreshold
			? alternateColor
			: baseColor;
		vec3 accumulatedColor = vec3(0.0);
		for (int i = 1; i < numOfTextures; ++i)
			accumulatedColor += blendingColor[i - 1] * texture(textures[i], vTexCoord[i]).rgb;
		fColor = vec4(accumulatedColor / (numOfTextures - 1), blendingColor.a) * color;
	}
}
