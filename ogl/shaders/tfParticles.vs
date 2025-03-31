#version 440

in vec3 bPos;
in vec4 bColor;
in vec4 bVelocityAndTime;
in vec3 bHSizeAndAngleAttribIdx;

out vec3 vPos;
out vec4 vColor;
out vec4 vVelocityAndTime;
out vec3 vHSizeAndAngleAttribIdx;

uniform float time;
uniform float deltaTime;
uniform vec2 lifeTimeRange;
uniform bool init;
uniform bool respawning;
uniform vec3 origin;
uniform vec3 initVelocity;
uniform vec2 velocitySpreadFactorRange;
uniform float velocityRotateZHRange;
uniform vec4 colorRange[2];
uniform vec3 gravity;

// A single iteration of Bob Jenkins' One-At-A-Time hashing algorithm.
uint hash( uint x ) {
	x += ( x << 10u );
	x ^= ( x >>  6u );
	x += ( x <<  3u );
	x ^= ( x >> 11u );
	x += ( x << 15u );
	return x;
}

// Compound versions of the hashing algorithm I whipped together.
uint hash( uint x, uint t )  { return hash( x ^ hash(t)                                       ); }
uint hash( uvec2 v, uint t ) { return hash( v.x ^ hash(v.y) ^ hash(t)                         ); }
uint hash( uvec3 v, uint t ) { return hash( v.x ^ hash(v.y) ^ hash(v.z) ^ hash(t)             ); }
uint hash( uvec4 v, uint t ) { return hash( v.x ^ hash(v.y) ^ hash(v.z) ^ hash(v.w) ^ hash(t) ); }

// Construct a float with half-open range [0:1] using low 23 bits.
// All zeroes yields 0.0, all ones yields the next smallest representable value below 1.0.
float floatConstruct( uint m ) {
	const uint ieeeMantissa = 0x007FFFFFu; // binary32 mantissa bitmask
	const uint ieeeOne      = 0x3F800000u; // 1.0 in IEEE binary32

	m &= ieeeMantissa;                     // Keep only mantissa bits (fractional part)
	m |= ieeeOne;                          // Add fractional part to 1.0

	float  f = uintBitsToFloat( m );       // Range [1:2]
	return f - 1.0;                        // Range [0:1]
}

// Pseudo-random value in half-open range [0:1].
float nRandom( float x, float t ) { return floatConstruct(hash(floatBitsToUint(x), floatBitsToUint(t))); }
float nRandom( vec2  v, float t ) { return floatConstruct(hash(floatBitsToUint(v), floatBitsToUint(t))); }
float nRandom( vec3  v, float t ) { return floatConstruct(hash(floatBitsToUint(v), floatBitsToUint(t))); }
float nRandom( vec4  v, float t ) { return floatConstruct(hash(floatBitsToUint(v), floatBitsToUint(t))); }

// float random(float x, float t) { return nRandom(x, t) * 2.0 - 1.0; }
// float random(vec2 v, float t) { return nRandom(v, t) * 2.0 - 1.0; }
// float random(vec3 v, float t) { return nRandom(v, t) * 2.0 - 1.0; }
// float random(vec4 v, float t) { return nRandom(v, t) * 2.0 - 1.0; }

float randomRange(vec2 range, float baseSeed)
{
	return nRandom(baseSeed, gl_VertexID) * (range.y - range.x) + range.x;
}

vec3 rotateZ(vec3 v, float angle) {
	const float s = sin(angle);
	const float c = cos(angle);
	const mat3 rotZ = mat3(
		c, -s, 0.0,
		s,  c, 0.0,
		0.0, 0.0, 1.0
	);
	return rotZ * v;
}

const float lifeTime = randomRange(lifeTimeRange, 123.0);

void velocitySpread(inout vec3 velocity)
{
	const float length = length(velocity);
	if (length == 0.0)
		return;

	velocity = rotateZ(velocity, randomRange(vec2(-velocityRotateZHRange, velocityRotateZHRange), 456.0 * time));
	velocity *= randomRange(velocitySpreadFactorRange, 789.0 * time);
}

void updateLifetimeRelatedState(inout vec3 inOutPosition, inout vec3 inOutVelocity, inout float inOutLifetime, inout vec4 inOutColor)
{
	if (init)
	{
		if (respawning)
		{
			inOutLifetime = randomRange(vec2(0.0, lifeTime), 101112.0 * time);
			inOutColor = vec4(0.0);
		}
		else
		{
			//inOutPosition = origin;
			//inOutVelocity = initVelocity;
			//inOutLifetime = randomRange(vec2(0.0, lifeTime), 101112.0 * time);
			//inOutColor = vec4(1.0);
			velocitySpread(inOutVelocity);
		}

		return;
	}

	inOutLifetime += deltaTime;

	if (lifeTimeRange.y > 0.0 && inOutLifetime >= lifeTime)
	{
		inOutPosition = origin;
		inOutVelocity = initVelocity;
		inOutLifetime = 0.0;
		inOutColor = respawning ? vec4(1.0) : vec4(0.0);
		velocitySpread(inOutVelocity);
	}

	if (inOutColor.a > 0.0)
	{
		inOutVelocity += gravity * deltaTime;
		inOutPosition += inOutVelocity * deltaTime;

		inOutColor = mix(mix(colorRange[0], colorRange[1], nRandom(161718.0, gl_VertexID)), vec4(0.0),
			lifeTimeRange.y > 0.0
			? inOutLifetime / lifeTime
			: 0.0);
	}
}

void main()
{
	vPos = bPos;
	vColor = bColor;
	vVelocityAndTime = bVelocityAndTime;

	updateLifetimeRelatedState(vPos, vVelocityAndTime.xyz, vVelocityAndTime.w, vColor);

	vHSizeAndAngleAttribIdx = bHSizeAndAngleAttribIdx;
}
