#version 440

in vec3 bPos;
in vec4 bColor;
in vec4 bVelocityAndTime;

out vec4 vColor;
out vec4 vVelocityAndTime;

void main()
{
	vColor = bColor;
	vVelocityAndTime = bVelocityAndTime;
	gl_Position = vec4(bPos, 1.0);
}
