#version 440

in vec3 bPos;

out vec2 vTexCoord;

uniform mat4 model = mat4(1.0);
uniform mat4 vp = mat4(1.0);
uniform vec2 textureTranslate = vec2(0.0);
uniform vec2 textureScale = vec2(1.0);
uniform bool textureCoordBasedOnModelTransform = false;

void main()
{
	vTexCoord = ((textureCoordBasedOnModelTransform ? model : mat4(1.0)) * vec4(bPos, 1.0)).xy / textureScale - textureTranslate;
	gl_Position = vp * model * vec4(bPos, 1.0);
}
