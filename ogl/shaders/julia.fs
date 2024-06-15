#version 440

in vec3 vPos;

out vec4 fColor;

uniform vec2 juliaC;
uniform vec2 juliaCOffset;
uniform vec4 minColor;
uniform vec4 maxColor;
uniform float zoom;
uniform vec2 translation;
uniform int iterations;

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
	return mix(maxColor, minColor, min(length(fractalResult), 1.0));
}

void main()
{
	fColor = gradientColoring(julia(vPos.xy / zoom + translation));
}
