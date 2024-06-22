#version 440

in vec3 vPos;
in vec4 vSmoothColor;
flat in vec4 vFlatColor;
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

float getLightAttenuation(int lightId)
{
	const float d = distance(vPos, lightsPos[lightId]) * lightsAttenuation[lightId];
	return 1.0 / (1.0 + d * d);
}

float getFogAmplification()
{
	const float d = distance(vPos, viewPos);
	return 1.0 - 1.0 / (1.0 + d * d * fogAmplification);
}

void main()
{
	const vec4 vColor = flatColor ? vFlatColor : vSmoothColor;

	if (numOfLights == 0 || !lightModelEnabled)
	{
		vec4 finalColor = color * vColor;
		finalColor.xyz = mix(finalColor.xyz, clearColor, getFogAmplification()); 
		finalColor += illumination;
		fColor = finalColor;
		return;
	}

	const vec3 normal = normalize(flatNormal ? vFlatNormal : vSmoothNormal);
	const vec3 viewDir = normalize(viewPos - vPos);
	const float frontFactor = step(0.0, dot(normal, viewDir));
	const vec3 partialDarkColor = darkColor / numOfLights;
	vec3 lightModelColor = vec3(0.0);

	for (int i = 0; i < numOfLights; ++i)
	{
		const vec3 lightDir = normalize(lightsPos[i] - vPos);
		lightModelColor += mix(partialDarkColor, partialDarkColor * lightsDarkColorFactor[i] + vColor.rgb * color.rgb * lightsCol[i] * (getAmbientFactor() + getDiffuseFactor(lightDir, normal, frontFactor))
			+ mix(vec3(1.0f), vColor.rgb * color.rgb, specularMaterialColorFactor) * lightsCol[i] * getSpecularFactor(lightDir, normal, viewDir, frontFactor), getLightAttenuation(i));
	}

	if (lightModelColorNormalization)
	{
		const float lightModelColorComponentMax = max(max(lightModelColor.r, lightModelColor.g), lightModelColor.b);

		if (lightModelColorComponentMax > 1.0)
			lightModelColor /= lightModelColorComponentMax;
	}

	fColor = vec4(mix(lightModelColor * vColor.a * color.a, clearColor, getFogAmplification()), vColor.a * color.a) + illumination;
}
