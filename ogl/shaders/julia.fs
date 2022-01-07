#version 440

in vec3 vPos;

out vec4 fColor;

uniform vec2 juliaCOffset;
uniform vec4 minColor;
uniform vec4 maxColor;

const int iterations = 200;
const uniform float zoom = 1.0;
const uniform vec2 translation = vec2(0.0, 0.0);
const uniform vec2 juliaC = vec2(-0.1, 0.65);

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
