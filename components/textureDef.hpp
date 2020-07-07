#include <string>

#include <GL/glew.h>

namespace Components
{
	struct TextureDef
	{
		TextureDef(std::string path, GLenum wrapMode = GL_CLAMP_TO_EDGE, GLenum minFilter = GL_LINEAR_MIPMAP_LINEAR,
			GLenum magFilter = GL_LINEAR_MIPMAP_LINEAR) : path(std::move(path)), wrapMode(wrapMode), minFilter(minFilter), magFilter(magFilter)
		{
		}

		std::string path;
		GLenum wrapMode;
		GLenum minFilter;
		GLenum magFilter;
	};
}
