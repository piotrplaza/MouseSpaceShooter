#pragma once

#include "_componentBase.hpp"

#include <ogl/shaders.hpp>

#include <functional>

namespace Components
{
	struct RenderingSetup : ComponentBase
	{
		RenderingSetup(std::function<std::function<void()>(Shaders::ProgramId)> setup) :
			setup(std::move(setup))
		{
		}

		std::function<std::function<void()>(Shaders::ProgramId)> setup;

		inline std::function<void()> operator()(Shaders::ProgramId programId)
		{
			return setup(programId);
		}
	};
}
