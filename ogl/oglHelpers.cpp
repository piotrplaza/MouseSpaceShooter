#include "oglHelpers.hpp"

#include <GL/glew.h>
#include <Windows.h>

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
}
