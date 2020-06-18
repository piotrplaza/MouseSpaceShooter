#version 440

in vec3 bPos;
in vec4 bColor;

out vec4 vColor;

uniform mat4 mvp = mat4(1.0);

void main()
{
	vColor = bColor;
	gl_Position = mvp * vec4(bPos, 1.0);
}
