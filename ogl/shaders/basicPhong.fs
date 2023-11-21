#version 440

in vec3 vPos;
in vec4 vColor;
in vec3 vNormal;

out vec4 fColor;

uniform vec4 color;
uniform int numOfLights;
uniform vec3 lightsPos[128];
uniform vec3 lightsCol[128];
uniform float lightsAttenuation[128];
uniform float ambient;
uniform float diffuse;

void main()
{
	vec3 lightModelColor = vec3(0.0);

	for (int i = 0; i < numOfLights; ++i)
	{
		const vec3 lightDir = normalize(lightsPos[i] - vPos);
		const float diffuseFactor = max(dot(normalize(vNormal), lightDir), 0.0) * diffuse;
		lightModelColor += vColor.rgb * lightsCol[i] * (diffuseFactor + ambient) * (1.0 / (1.0 + lightsAttenuation[i] * distance(vPos, lightsPos[i])));
	}

	const float lightModelColorComponentMax = max(max(lightModelColor.r, lightModelColor.g), lightModelColor.b);

	if (lightModelColorComponentMax > 1.0)
	{
		lightModelColor /= lightModelColorComponentMax;
	}

	fColor = vec4(lightModelColor, vColor.a) * color;
}
