#version 440

in vec3 bPos;
in vec4 bColor;
layout(location = 4) in mat4 bInstanceTransform;

out vec4 vColor;

uniform mat4 model;
uniform mat4 vp;

void main()
{
	vColor = bColor;
	gl_Position = vp * bInstanceTransform * model * vec4(bPos, 1.0);
}
