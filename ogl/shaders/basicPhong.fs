#version 440

in vec3 vPos;
in vec4 vSmoothColor;
flat in vec4 vFlatColor;
in vec3 vSmoothNormal;
flat in vec3 vFlatNormal;

out vec4 fColor;

uniform vec4 color;
uniform int numOfLights;
uniform vec3 lightsPos[128];
uniform vec3 lightsCol[128];
uniform float lightsAttenuation[128];
uniform float ambient;
uniform float diffuse;
uniform vec3 viewPos;
uniform float specular;
uniform float specularFocus;
uniform bool flatColor;
uniform bool flatNormal;
uniform bool lightModelColorNormalization;

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
	return 1.0 / (1.0 + lightsAttenuation[lightId] * distance(vPos, lightsPos[lightId]));
}

void main()
{
	const vec4 vColor = flatColor ? vFlatColor : vSmoothColor;

	if (numOfLights == 0)
	{
		fColor = color * vColor;
		return;
	}

	const vec3 normal = normalize(flatNormal ? vFlatNormal : vSmoothNormal);
	vec3 lightModelColor = vec3(0.0);

	for (int i = 0; i < numOfLights; ++i)
	{
		const vec3 lightDir = normalize(lightsPos[i] - vPos);
		lightModelColor += getAttenuation(i) * (vColor.rgb * lightsCol[i] * (getAmbientFactor() + getDiffuseFactor(lightDir, normal))
			+ lightsCol[i] * getSpecularFactor(lightDir, normal));
	}

	if (lightModelColorNormalization)
	{
		const float lightModelColorComponentMax = max(max(lightModelColor.r, lightModelColor.g), lightModelColor.b);

		if (lightModelColorComponentMax > 1.0)
			lightModelColor /= lightModelColorComponentMax;
	}

	fColor = vec4(lightModelColor, vColor.a) * color;
}
