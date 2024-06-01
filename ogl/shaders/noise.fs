#version 440

in vec4 vColor;

out vec4 fColor;

uniform vec4 color;
uniform bool separateNoises;
uniform vec4 noiseWeights;
uniform vec4 noiseMin;
uniform vec4 noiseMax;
uniform float time;

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
float random( float x, float t = 0.0 ) { return floatConstruct(hash(floatBitsToUint(x), floatBitsToUint(t))); }
float random( vec2  v, float t = 0.0 ) { return floatConstruct(hash(floatBitsToUint(v), floatBitsToUint(t))); }
float random( vec3  v, float t = 0.0 ) { return floatConstruct(hash(floatBitsToUint(v), floatBitsToUint(t))); }
float random( vec4  v, float t = 0.0 ) { return floatConstruct(hash(floatBitsToUint(v), floatBitsToUint(t))); }

void main()
{
	const vec4 noise = ((separateNoises 
		? vec4(random(vec3(gl_FragCoord.xy, 0.0), time), random(vec3(gl_FragCoord.xy, 1.0), time), random(vec3(gl_FragCoord.xy, 2.0), time), random(vec3(gl_FragCoord.xy, 3.0), time))
		: vec4(random(vec3(gl_FragCoord.xy, 0.0), time))) * (noiseMax - noiseMin) + noiseMin) * noiseWeights;

	fColor = vColor * color + noise;
}
