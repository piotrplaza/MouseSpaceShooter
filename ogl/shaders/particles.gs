#version 440

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

in float vAngle[];
in vec4 vColor[];

out vec4 gColor;
out vec2 gTexCoord;

uniform mat4 vp;

mat4 rotationMatrix(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    
    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
}

void main()
{
    const vec4 position = vec4(gl_in[0].gl_Position.xy, 0.0, 1.0);
    const float hSize = 1.0 * gl_in[0].gl_Position.z;
    const mat4 rotMat = rotationMatrix(vec3(0.0, 0.0, 1.0), vAngle[0]);

	gl_Position = vp * (position + rotMat * vec4(-hSize, -hSize, 0.0, 0.0));
    gColor = vColor[0];
    gTexCoord = vec2(0.0, 0.0);
    EmitVertex();
    gl_Position = vp * (position + rotMat * vec4(hSize, -hSize, 0.0, 0.0));
    gColor = vColor[0];
    gTexCoord = vec2(1.0, 0.0);
    EmitVertex();
    gl_Position = vp * (position + rotMat * vec4(-hSize, hSize, 0.0, 0.0));
    gColor = vColor[0];
    gTexCoord = vec2(0.0, 1.0);
    EmitVertex();
    gl_Position = vp * (position + rotMat * vec4(hSize, hSize, 0.0, 0.0));
    gColor = vColor[0];
    gTexCoord = vec2(1.0, 1.0);
    EmitVertex();

    EndPrimitive();
}
