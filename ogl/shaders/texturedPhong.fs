#version 440

in vec3 vPos;
in vec4 vColor;
in vec2 vTexCoord[5];
in vec3 vNormal;

out vec4 fColor;

uniform vec4 color;
uniform vec3 clearColor;
uniform int numOfLights;
uniform vec3 lightsPos[128];
uniform vec3 lightsCol[128];
uniform float lightsAttenuation[128];
uniform float ambient;
uniform float diffuse;
uniform vec3 viewPos;
uniform float specular;
uniform float specularFocus;
uniform bool lightModelEnabled;
uniform vec4 mulBlendingColor;
uniform vec4 addBlendingColor;
uniform int numOfTextures;
uniform sampler2D textures[5];
uniform bool alphaFromBlendingTexture;
uniform bool colorAccumulation;

float getAmbientFactor()
{
	return ambient;
}

float getDiffuseFactor(const vec3 lightDir, const vec3 normal)
{
	return abs(dot(normal, lightDir)) * diffuse;
}

float getSpecularFactor(const vec3 lightDir, const vec3 normal)
{
	const vec3 viewDir = normalize(viewPos - vPos);
	const vec3 reflectDir = reflect(-lightDir, normal);
	return pow(max(dot(viewDir, reflectDir), 0.0), specularFocus) * specular;
}

float getAttenuation(int lightId)
{
	const float d = distance(vPos, lightsPos[lightId]) * lightsAttenuation[lightId];
	return 1.0 / (1.0 + d * d);
}

void main()
{
	const vec3 normal = normalize(vNormal);
	vec3 lightModelColor = vec3(0.0);

	if (lightModelEnabled)
	{
		for (int i = 0; i < numOfLights; ++i)
		{
			const vec3 lightDir = normalize(lightsPos[i] - vPos);
			lightModelColor += mix(clearColor, vec3(1.0), getAttenuation(i)) * vColor.rgb * lightsCol[i] * (getAmbientFactor() + getDiffuseFactor(lightDir, normal))
				+ getAttenuation(i) * lightsCol[i] * getSpecularFactor(lightDir, normal);
		}
	}

	if (numOfTextures == 1)
		fColor = vec4(lightModelColor, vColor.a) * texture(textures[0], vTexCoord[0]) * vColor * color;
	else if (numOfTextures == 0)
		fColor = vec4(lightModelColor, vColor.a) * vColor * color;
	else
	{
		const vec4 finalBlendingColor = texture(textures[0], vTexCoord[0]) * mulBlendingColor + addBlendingColor;
		vec4 accumulatedColor = vec4(0.0);

		for (int i = 1; i < numOfTextures; ++i)
			accumulatedColor += finalBlendingColor[i - 1] * texture(textures[i], vTexCoord[i]);

		fColor = vec4(lightModelColor, vColor.a) * vec4((accumulatedColor / (colorAccumulation ? 1 : (numOfTextures - 1))).rgb,
			alphaFromBlendingTexture ? finalBlendingColor.a : accumulatedColor.a
		) * vColor * color;
	}
}
