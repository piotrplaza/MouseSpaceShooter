#version 440

in vec3 bPos;
in vec4 bColor;
layout(location = 6) in mat4 bInstancedTransform;

out vec4 vColor;

uniform mat4 model;
uniform mat4 vp;

void main()
{
	vColor = bColor;
	gl_Position = vp * bInstancedTransform * model * vec4(bPos, 1.0);
}
