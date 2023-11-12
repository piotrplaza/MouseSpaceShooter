#version 440

in vec3 vPos;
in vec4 vColor;
in vec2 vTexCoord[5];
in vec3 vNormal;

out vec4 fColor;

uniform vec4 color;
uniform int numOfLights;
uniform vec3 lightsPos[128];
uniform vec3 lightsCol[128];
uniform vec4 mulBlendingColor;
uniform vec4 addBlendingColor;
uniform int numOfTextures;
uniform sampler2D textures[5];
uniform bool alphaFromBlendingTexture;
uniform bool colorAccumulation;

void main()
{
	const vec3 ambient = 0.1 * vColor.rgb;
	vec3 result = ambient;

	for (int i = 0; i < numOfLights; ++i)
	{
		const vec3 lightDir = normalize(lightsPos[i] - vPos);
		const float diff = max(dot(vNormal, lightDir), 0.0);
		const vec3 diffuse = diff * lightsCol[i];

		result += diffuse;
	}

	if (numOfTextures == 1)
		fColor = vec4(result, vColor.a) * texture(textures[0], vTexCoord[0]) * vColor * color;
	else if (numOfTextures == 0)
		fColor = vec4(result, vColor.a) * vColor * color;
	else
	{
		const vec4 finalBlendingColor = texture(textures[0], vTexCoord[0]) * mulBlendingColor + addBlendingColor;
		vec4 accumulatedColor = vec4(0.0);

		for (int i = 1; i < numOfTextures; ++i)
			accumulatedColor += finalBlendingColor[i - 1] * texture(textures[i], vTexCoord[i]);

		fColor = vec4(result, vColor.a) * vec4((accumulatedColor / (colorAccumulation ? 1 : (numOfTextures - 1))).rgb,
			alphaFromBlendingTexture ? finalBlendingColor.a : accumulatedColor.a
		) * vColor * color;
	}
}
