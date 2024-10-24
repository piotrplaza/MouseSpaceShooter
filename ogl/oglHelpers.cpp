#include "oglHelpers.hpp"

#include "oglProxy.hpp"

#include <Windows.h>

#include <iostream>

namespace Tools
{
	ConditionalScopedFramebuffer::ConditionalScopedFramebuffer(bool cond, unsigned fbo, glm::ivec2 localFbViewportSize, unsigned defaultFBO, glm::ivec2 defaultFbViewportSize):
		cond(cond),
		defaultFBO(defaultFBO),
		defaultFbViewportSize(defaultFbViewportSize)
	{
		if (!cond)
			return;

		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glViewport(0, 0, localFbViewportSize.x, localFbViewportSize.y);
	}

	ConditionalScopedFramebuffer::~ConditionalScopedFramebuffer()
	{
		if (!cond)
			return;

		glBindFramebuffer(GL_FRAMEBUFFER, defaultFBO);
		glViewport(0, 0, defaultFbViewportSize.x, defaultFbViewportSize.y);
	}

	void VSync(bool enabled)
	{
		typedef BOOL(APIENTRY* PFNWGLSWAPINTERVALPROC)(int);
		PFNWGLSWAPINTERVALPROC wglSwapIntervalEXT = 0;

		const char* extensions = (char*)glGetString(GL_EXTENSIONS);

		if (strstr(extensions, "WGL_EXT_swap_control") == 0)
		{
			return;
		}
		else
		{
			wglSwapIntervalEXT = (PFNWGLSWAPINTERVALPROC)wglGetProcAddress("wglSwapIntervalEXT");

			if (wglSwapIntervalEXT)
			{
				wglSwapIntervalEXT(enabled);
			}
		}
	}

	void PrintGLErrors(const std::string& prefix)
	{
		GLenum err = GL_NO_ERROR;
		while ((err = glGetError()) != GL_NO_ERROR)
		{
			std::cout << prefix << ": 0x" << std::hex << err << std::endl;
		}
	}

	void PrintGLErrorsOnce(const std::string& prefix)
	{
		static bool printed = false;
		if (!printed)
		{
			PrintGLErrors(prefix);
			printed = true;
		}
	}

	int GetPixelFormat(int numOfChannels)
	{
		switch (numOfChannels)
		{
		case 1: return GL_RED;
		case 2: return GL_RG;
		case 3: return GL_RGB;
		case 4: return GL_RGBA;
		default: assert(!"unsupported number of channels"); return 0;
		}
	}
}
