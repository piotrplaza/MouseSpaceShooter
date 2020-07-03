#pragma once

#include <shaders.hpp>

namespace Shaders
{
	namespace Programs
	{
		struct SceneCoordTextured
		{
			Shaders::ProgramId program;
			Shaders::UniformId mvpUniform;
			Shaders::UniformId modelUniform;
			Shaders::UniformId texture1Uniform;
			Shaders::UniformId textureScalingUniform;
		};
	}
}
