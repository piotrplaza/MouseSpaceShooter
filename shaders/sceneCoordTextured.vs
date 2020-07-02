#version 440

in vec3 bPos;

out vec2 vTexCoord;

uniform mat4 mvp = mat4(1.0);
uniform mat4 model = mat4(1.0);
uniform vec2 textureScaling = vec2(1.0);

void main()
{
	vTexCoord = (model * vec4(bPos, 1.0)).xy * textureScaling;
	gl_Position = mvp * vec4(bPos, 1.0);
}
