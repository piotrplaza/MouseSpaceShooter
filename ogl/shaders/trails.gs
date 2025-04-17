#version 440

layout (points) in;
layout (line_strip, max_vertices = 2) out;

in vec4 vColor[];
in vec4 vVelocityAndTime[];

out vec4 gColor;

uniform mat4 vp;
uniform vec4 color;
uniform float deltaTime;

void main()
{
    gl_Position = vp * gl_in[0].gl_Position;
    gColor = vColor[0] * color;
    EmitVertex();
    gl_Position = vp * (gl_in[0].gl_Position + vec4(vVelocityAndTime[0].xyz * deltaTime, 0.0));
    gColor = vColor[0] * color;
    EmitVertex();

    EndPrimitive();
}
