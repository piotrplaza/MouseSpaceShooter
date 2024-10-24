#pragma once

#include <glm/vec2.hpp>

#include <string>

namespace Tools
{
	class ConditionalScopedFramebuffer
	{
	public:
		ConditionalScopedFramebuffer(bool cond, unsigned fbo, glm::ivec2 localFbViewportSize, unsigned defaultFBO, glm::ivec2 defaultFbViewportSize);
		~ConditionalScopedFramebuffer();

	private:
		const bool cond;
		const unsigned defaultFBO;
		const glm::ivec2 defaultFbViewportSize;
	};

	void VSync(bool enabled);
	void PrintGLErrors(const std::string& prefix = "glError");
	void PrintGLErrorsOnce(const std::string& prefix = "glError");
	int GetPixelFormat(int numOfChannels);
}
