#version 440

in vec3 bPos;
in vec4 bColor;
in vec4 bVelocityAndTime;
in vec3 bHSizeAndAngleAttribIdx;

out vec3 vPos;
out vec4 vColor;
out vec4 vVelocityAndTime;
out vec3 vHSizeAndAngleAttribIdx;

uniform float deltaTime;

void main()
{
	vPos = bPos + bVelocityAndTime.xyz * deltaTime;
	vColor = bColor + 0.01 * deltaTime;
	vVelocityAndTime = bVelocityAndTime;
	vHSizeAndAngleAttribIdx = bHSizeAndAngleAttribIdx;
}
