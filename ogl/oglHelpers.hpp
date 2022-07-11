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
	void PrintCurrentGLErrors(const std::string& prefix);
}
