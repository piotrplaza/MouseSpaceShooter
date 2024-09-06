#version 440

in vec3 bPos;
in vec4 bColor;

out vec3 vPos;
out vec4 vColor;

void main()
{
	vPos = bPos;
	vColor = bColor;
}
