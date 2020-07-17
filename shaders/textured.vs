#version 440

in vec3 bPos;
in vec2 bTexCoord;

out vec2 vTexCoord;

uniform mat4 mvp = mat4(1.0);
uniform vec2 textureTranslate = vec2(0.0);
uniform vec2 textureScale = vec2(1.0);

void main()
{
	vTexCoord = bTexCoord / textureScale - textureTranslate;
	gl_Position = mvp * vec4(bPos, 1.0);
}
