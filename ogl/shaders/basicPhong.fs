#version 440

in vec3 vPos;
in vec4 vColor;
in vec3 vNormal;

out vec4 fColor;

uniform vec4 color;
uniform int numOfLights;
uniform vec3 lightsPos[128];
uniform vec3 lightsCol[128];

void main()
{
	const vec3 ambient = 0.1 * vColor.rgb;
	vec3 result = ambient;

	for (int i = 0; i < numOfLights; ++i)
	{
		const vec3 lightDir = normalize(lightsPos[i] - vPos);
		const float diff = max(dot(normalize(vNormal), lightDir), 0.0);
		const vec3 diffuse = diff * lightsCol[i] * vColor.rgb * 0.8;

		result += diffuse;
	}

	fColor = vec4(result, vColor.a) * color;
}
