#version 440

in vec3 vPos;

out vec4 fColor;

uniform int iterations = 200;
uniform float zoom = 1.0;
uniform vec2 translation = vec2(0.0, 0.0);
uniform vec2 juliaC = vec2(-0.1, 0.65);
uniform vec2 juliaCOffset = vec2(0.0, 0.0);
uniform vec4 minColor = vec4(0.0, 0.0, 0.0, 1.0);
uniform vec4 maxColor = vec4(1.0, 1.0, 1.0, 1.0);

vec2 complexAdd(vec2 c1, vec2 c2)
{
	return vec2(c1.x + c2.x, c1.y + c2.y);
}

vec2 complexMul(vec2 c1, vec2 c2)
{
	return vec2(c1.x * c2.x - c1.y * c2.y, c1.x * c2.y + c1.y * c2.x);
}

vec2 julia(vec2 p)
{
	vec2 result = p;
	for (int i = 0; i < iterations; ++i)
	{
		result = complexAdd(complexMul(result, result), juliaC + juliaCOffset);
	}
	return result;
}

vec4 gradientColoring(vec2 fractalResult)
{
	return normalize(mix(minColor, maxColor, length(fractalResult)));
}

void main()
{
	fColor = gradientColoring(julia(vPos.xy / zoom + translation));
}
