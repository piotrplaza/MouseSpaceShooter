#include <string>

#include <GL/glew.h>

namespace Components
{
	struct Texture
	{
		Texture()
		{
			glGenTextures(1, &textureObject);
		}

		~Texture()
		{
			delete[] bytes;
			glDeleteTextures(1, &textureObject);
		}

		unsigned textureUnit = 0;
		unsigned textureObject = 0;
		unsigned char* bytes = nullptr;
		int width = 0;
		int height = 0;
		int bitDepth = 0;
	};
}
