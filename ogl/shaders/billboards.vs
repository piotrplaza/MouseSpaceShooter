#version 440

in vec3 bPos;
in vec4 bColor;

out float vAngle;
out vec4 vColor;

uint StableRandom(uint seed)
{
	uint i = (seed ^ 12345391u) * 2654435769u;
	i ^= (i << 6u) ^ (i >> 26u);
	i *= 2654435769u;
	i += (i << 5u) ^ (i >> 12u);
	return i;
}

float randhashf(uint seed, float b)
{
	return float(b * StableRandom(seed)) / 4294967295u;
}

#define PI 3.1415926538

void main()
{
	vAngle = randhashf(uint(gl_VertexID), 2 * PI);
	vColor = bColor;
	gl_Position = vec4(bPos, 1.0);
}
