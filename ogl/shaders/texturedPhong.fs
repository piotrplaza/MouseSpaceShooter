#version 440

in vec3 vPos;
in vec4 vSmoothColor;
flat in vec4 vFlatColor;
in vec2 vTexCoord[5];
in vec3 vSmoothNormal;
flat in vec3 vFlatNormal;

out vec4 fColor;

uniform vec4 color;
uniform vec3 clearColor;
uniform int numOfLights;
uniform vec3 lightsPos[128];
uniform vec3 lightsCol[128];
uniform float lightsAttenuation[128];
uniform float lightsDarkColorFactor[128];
uniform float ambient;
uniform float diffuse;
uniform vec3 viewPos;
uniform float specular;
uniform float specularFocus;
uniform float specularMaterialColorFactor;
uniform vec4 illumination;
uniform vec3 darkColor;
uniform bool flatColor;
uniform bool flatNormal;
uniform bool lightModelColorNormalization;
uniform bool lightModelEnabled;
uniform vec4 mulBlendingColor;
uniform vec4 addBlendingColor;
uniform int numOfTextures;
uniform sampler2D textures[5];
uniform bool alphaFromBlendingTexture;
uniform bool colorAccumulation;
uniform int numOfPlayers;
uniform vec3 playerCenter;
uniform float playerUnhidingRadius;
uniform bool visibilityReduction;
uniform vec3 visibilityCenter;
uniform float fullVisibilityDistance;
uniform float invisibilityDistance;
uniform float alphaDiscardTreshold;
uniform float fogAmplification;

float getAmbientFactor()
{
	return ambient;
}

float getDiffuseFactor(const vec3 lightDir, const vec3 normal, const float frontFactor)
{
	return max(dot(normal, lightDir) * diffuse * frontFactor, 0.0f);
}

float getSpecularFactor(const vec3 lightDir, const vec3 normal, const vec3 viewDir, const float frontFactor) {
	const vec3 reflectDir = reflect(-lightDir, normal);
	return pow(max(dot(viewDir, reflectDir), 0.0), specularFocus) * specular * frontFactor;
}

float getAttenuation(int lightId)
{
	const float d = distance(vPos, lightsPos[lightId]) * lightsAttenuation[lightId];
	return 1.0 / (1.0 + d * d);
}

float playersDistanceAlpha()
{
	float minDist = 1000000;
	for (int i = 0; i < numOfPlayers; ++i)
		minDist = min(minDist, distance(vPos, playerCenter));

	return clamp(minDist / playerUnhidingRadius, 0.0, 1.0);
}

float visibility()
{
	if (!visibilityReduction)
		return playersDistanceAlpha();

	const float dist = distance(visibilityCenter, vPos);
	const float visibilityFactor = clamp((dist - invisibilityDistance) / -max(invisibilityDistance - fullVisibilityDistance, 0.001), 0.0, 1.0);
	return playersDistanceAlpha() * visibilityFactor;
}

vec4 lightModel(vec4 inColor)
{
	if (numOfLights == 0 || !lightModelEnabled)
	{
		return inColor * color;
	}

	const vec3 normal = normalize(flatNormal ? vFlatNormal : vSmoothNormal);
	const vec3 viewDir = normalize(viewPos - vPos);
	const float frontFactor = step(0.0, dot(normal, viewDir));
	const vec3 partialDarkColor = darkColor / numOfLights;
	vec3 lightModelColor = vec3(0.0);

	for (int i = 0; i < numOfLights; ++i)
	{
		const vec3 lightDir = normalize(lightsPos[i] - vPos);
		lightModelColor += mix(partialDarkColor, partialDarkColor * lightsDarkColorFactor[i] + inColor.rgb * color.rgb * lightsCol[i] * (getAmbientFactor() + getDiffuseFactor(lightDir, normal, frontFactor))
			+ mix(vec3(1.0f), inColor.rgb * color.rgb, specularMaterialColorFactor) * lightsCol[i] * getSpecularFactor(lightDir, normal, viewDir, frontFactor), getAttenuation(i));
	}

	if (lightModelColorNormalization)
	{
		const float lightModelColorComponentMax = max(max(lightModelColor.r, lightModelColor.g), lightModelColor.b);

		if (lightModelColorComponentMax > 1.0)
			lightModelColor /= lightModelColorComponentMax;
	}

	return vec4(lightModelColor * inColor.a * color.a, inColor.a * color.a);
}

vec4 texturing(vec4 inColor)
{
	if (numOfTextures == 1)
		return texture(textures[0], vTexCoord[0]) * inColor;
	else if (numOfTextures == 0)
		return inColor;
	else
	{
		const vec4 finalBlendingColor = texture(textures[0], vTexCoord[0]) * mulBlendingColor + addBlendingColor;
		vec4 accumulatedColor = vec4(0.0);

		for (int i = 1; i < numOfTextures; ++i)
			accumulatedColor += finalBlendingColor[i - 1] * texture(textures[i], vTexCoord[i]);

		return vec4((accumulatedColor / (colorAccumulation ? 1 : (numOfTextures - 1))).rgb,
			alphaFromBlendingTexture ? finalBlendingColor.a : accumulatedColor.a) * inColor;
	}
}

float getFogAmplification()
{
	const float d = distance(vPos, viewPos);
	return 1.0 - 1.0 / (1.0 + d * d * fogAmplification);
}

void main()
{
	const vec4 vColor = flatColor ? vFlatColor : vSmoothColor;
	const vec4 texturedColor = texturing(vColor);

	if (alphaDiscardTreshold > 0.0 && texturedColor.a + illumination.a <= alphaDiscardTreshold)
		discard;

	vec4 finalColor = lightModel(texturedColor);
	finalColor.xyz = mix(finalColor.xyz, clearColor, getFogAmplification());
	finalColor += illumination;
	finalColor *= visibility();

	fColor = finalColor;
}
