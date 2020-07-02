#include <string>

#include <GL/glew.h>

namespace Components
{
	struct Texture
	{
		Texture(std::string path, GLenum wrapMode = GL_CLAMP_TO_EDGE, GLenum minFilter = GL_LINEAR_MIPMAP_LINEAR,
			GLenum magFilter = GL_LINEAR_MIPMAP_LINEAR) : path(std::move(path)), wrapMode(wrapMode), minFilter(minFilter), magFilter(magFilter)
		{
		}

		~Texture()
		{
			delete[] bytes;
			glDeleteTextures(1, &textureObject);
		}

		std::string path;
		GLenum wrapMode;
		GLenum minFilter;
		GLenum magFilter;

		unsigned textureUnit = 0;
		unsigned textureObject = 0;
		unsigned char* bytes = nullptr;
		int width = 0;
		int height = 0;
		int bitDepth = 0;
	};
}
