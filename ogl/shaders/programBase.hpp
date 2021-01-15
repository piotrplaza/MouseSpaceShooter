#pragma once

#include <ogl/shaders.hpp>
#include <ogl/uniformControllers.hpp>

namespace Shaders
{
	struct ProgramBase
	{
		ProgramBase() :
			program(0)
		{}

		ProgramBase(Shaders::ProgramId program) :
			program(program)
		{}

		bool isValid() const
		{
			return program != 0;
		}

		Shaders::ProgramId program;
	};
}
