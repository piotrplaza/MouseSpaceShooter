#version 440

layout(location = 0) in vec3 bPos;
layout(location = 2) in vec2 bTexCoord;

out vec3 vPos;
out vec2 vTexCoord;

void main()
{
	vPos = bPos;
	vTexCoord = bTexCoord;
	gl_Position = vec4(bPos, 1.0);
}
