#version 440

in vec3 bPos;
in vec4 bColor;
layout(location = 3) in vec3 bNormal;
layout(location = 4) in mat4 bInstanceTransform;

out vec3 vPos;
out vec4 vSmoothColor;
flat out vec4 vFlatColor;
out vec3 vSmoothNormal;
flat out vec3 vFlatNormal;

uniform mat4 model;
uniform mat4 vp;
uniform mat3 normalMatrix;

void main()
{
	vPos = vec3(bInstanceTransform * model * vec4(bPos, 1.0));

	vSmoothColor = bColor;
	vFlatColor = bColor;

	const vec3 normal = transpose(inverse(mat3(bInstanceTransform))) * normalMatrix * bNormal; // TODO: transpose(inverse(mat3(bInstanceTransform))) can be optimized, e.g. calc on CPU side and pass as another instanced attribute.
	vSmoothNormal = normal;
	vFlatNormal = normal;

	gl_Position = vp * vec4(vPos, 1.0);
}
