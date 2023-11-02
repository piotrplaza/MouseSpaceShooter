#version 440

in vec3 bPos;
in vec4 bColor;
in vec3 bNormal;

out vec3 vPos;
out vec4 vColor;
out vec3 vNormal;

uniform mat4 model;
uniform mat4 mv;
uniform mat4 vp;
uniform mat3 mvr;

void main()
{
	vPos = (mv * vec4(bPos, 1.0)).xyz;
	vColor = bColor;
	vNormal = mvr * bNormal;

	gl_Position = vp * model * vec4(bPos, 1.0);
}
