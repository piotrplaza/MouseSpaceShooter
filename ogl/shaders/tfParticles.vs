#version 440

in vec3 bPos;
in vec4 bColor;
in vec4 bVelocityAndTime;
in vec3 bHSizeAndAngleAttribIdx;

out vec3 vPos;
out vec4 vColor;
out vec4 vVelocityAndTime;
out vec3 vHSizeAndAngleAttribIdx;

uniform float deltaTime;
uniform vec2 velocitySpreadRange;

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
float random( float x, float t ) { return floatConstruct(hash(floatBitsToUint(x), floatBitsToUint(t))); }
float random( vec2  v, float t ) { return floatConstruct(hash(floatBitsToUint(v), floatBitsToUint(t))); }
float random( vec3  v, float t ) { return floatConstruct(hash(floatBitsToUint(v), floatBitsToUint(t))); }
float random( vec4  v, float t ) { return floatConstruct(hash(floatBitsToUint(v), floatBitsToUint(t))); }

float velocitySpreadRangeRandom(float base)
{
	return random(base, gl_VertexID) * (velocitySpreadRange.y - velocitySpreadRange.x) + velocitySpreadRange.x;
}

vec3 velocitySpread()
{
	if (velocitySpreadRange == vec2(0.0))
		return vec3(0.0);

	vec3 result = bVelocityAndTime.xyz;
	const float length = length(result);
	if (length > 0.0)
	{
		const float spread = velocitySpreadRangeRandom(111.0);
		result /= length;
		result *= spread;
	}
	return result;
}

void main()
{
	vPos = bPos + bVelocityAndTime.xyz * deltaTime;
	vColor = bColor + 0.01 * deltaTime;
	vVelocityAndTime = bVelocityAndTime + vec4(velocitySpread(), 0.0);
	vHSizeAndAngleAttribIdx = bHSizeAndAngleAttribIdx;
}
