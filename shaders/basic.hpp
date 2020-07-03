#pragma once

#include <shaders.hpp>

namespace Shaders
{
	namespace Programs
	{
		struct Basic
		{
			Shaders::ProgramId program;
			Shaders::UniformId mvpUniform;
			Shaders::UniformId colorUniform;
		};
	}
}
