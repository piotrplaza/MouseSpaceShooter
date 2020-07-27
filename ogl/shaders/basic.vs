#version 440

in vec3 bPos;

uniform mat4 model = mat4(1.0);
uniform mat4 vp = mat4(1.0);

void main()
{
	gl_Position = vp * model * vec4(bPos, 1.0);
}
