#version 440

in vec3 bPos;
in vec4 bColor;
in vec4 bVelocityAndTime;
in vec3 bHSizeAndAngleAttribIdx;

out vec3 vPos;
out vec4 vColor;
out vec4 vVelocityAndTime;
out vec3 vHSizeAndAngleAttribIdx;

uniform int componentId;
uniform float time;
uniform float deltaTime;
uniform int particlesCount;
uniform vec2 lifeTimeRange;
uniform bool init;
uniform bool respawning;
uniform bool originInit;
uniform vec3 originBegin;
uniform vec3 originEnd;
uniform float originForce;
uniform vec3 velocityOffset;
uniform vec3 initVelocity;
uniform vec2 velocitySpreadFactorRange;
uniform float velocityRotateZHRange;
uniform float velocityFactor;
uniform vec4 colorRange[2];
uniform vec3 globalForce;
uniform vec3 AZPlusBPlusCT;

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

vec3 origin = particlesCount == 0
	? originEnd
	: mix(originBegin, originEnd, float(gl_VertexID + 1) / particlesCount);

float lifeTime = randomRange(lifeTimeRange, 123.0 * componentId);
float seed = time * componentId;

void velocitySpread(inout vec3 velocity)
{
	const float length = length(velocity);
	if (length == 0.0)
		return;

	velocity = rotateZ(velocity, randomRange(vec2(-velocityRotateZHRange, velocityRotateZHRange), 456.0 * seed));
	velocity *= randomRange(velocitySpreadFactorRange, 789.0 * seed);
	velocity += velocityOffset;
}

void updateLifetimeRelatedState(inout vec3 inOutPosition, inout vec3 inOutVelocity, inout float inOutLifetime, inout vec4 inOutColor)
{
	if (init)
	{
		if (originInit)
			inOutPosition = origin;

		if (respawning)
		{
			inOutLifetime = randomRange(vec2(0.0, lifeTime), 101112.0 * seed);
			inOutColor = vec4(0.0);
		}
		else
			velocitySpread(inOutVelocity);

		return;
	}

	const vec4 baseColor = mix(colorRange[0], colorRange[1], nRandom(161718.0, gl_VertexID * componentId));

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
		inOutVelocity += (inOutVelocity * velocityFactor - inOutVelocity) * deltaTime;
		inOutVelocity += globalForce * deltaTime;

		if (originForce > 0.0)
		{
			const vec3 originDelta = inOutPosition - origin;
			const float originDist = length(originDelta);
			const vec3 originDir = originDist == 0.0 ? vec3(0.0) : normalize(originDelta);
			inOutVelocity -= originDir * originForce * deltaTime;
		}

		inOutPosition += inOutVelocity * deltaTime;

		inOutColor = mix(baseColor, vec4(0.0),
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
	vPos.z = AZPlusBPlusCT.x * vPos.z + AZPlusBPlusCT.y + AZPlusBPlusCT.z * vVelocityAndTime.w;

	vHSizeAndAngleAttribIdx = bHSizeAndAngleAttribIdx;
}
