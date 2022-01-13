#version 440

in vec2 vTexCoord[5];

out vec4 fColor;

uniform vec4 color;
uniform int numOfTextures;
uniform sampler2D textures[5];

void main()
{
	if (numOfTextures == 1)
		fColor = texture(textures[0], vTexCoord[0]) * color;
	else
	{
		const vec4 blendingColor = texture(textures[0], vTexCoord[0]);
		vec4 accumulatedColor = vec4(0.0);
		for (int i = 1; i < numOfTextures; ++i)
			accumulatedColor += blendingColor[i - 1] * texture(textures[i], vTexCoord[i]);
		fColor = vec4((accumulatedColor / (numOfTextures - 1)).rgb, accumulatedColor.a) * color;
	}
}
