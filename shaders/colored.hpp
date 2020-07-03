#pragma once

#include <shaders.hpp>

namespace Shaders
{
	namespace Programs
	{
		struct Colored
		{
			Shaders::ProgramId program;
			Shaders::UniformId mvpUniform;
		};
	}
}
