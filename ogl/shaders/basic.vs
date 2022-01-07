#version 440

in vec3 bPos;

uniform mat4 model;
uniform mat4 vp;

void main()
{
	gl_Position = vp * model * vec4(bPos, 1.0);
}
