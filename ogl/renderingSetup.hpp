#pragma once

#include <memory>

namespace Tools
{
	using RenderingSetup = std::function<std::function<void()>(Shaders::ProgramId)>;
	using UniqueRenderingSetup = std::unique_ptr<RenderingSetup>;

	template <typename... Args>
	inline UniqueRenderingSetup MakeUniqueRenderingSetup(Args&&... args)
	{
		return std::make_unique<RenderingSetup>(std::forward<Args>(args)...);
	}
}
