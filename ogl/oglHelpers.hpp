#include <glm/vec2.hpp>

namespace Tools
{
	class ConditionalScopedFramebuffer
	{
	public:
		ConditionalScopedFramebuffer(bool cond, unsigned fbo, glm::ivec2 localFbViewportSize, glm::ivec2 defaultFbViewportSize);
		~ConditionalScopedFramebuffer();

	private:
		const bool cond;
		const glm::ivec2 defaultFbViewportSize;
	};

	void VSync(bool enabled);
}
