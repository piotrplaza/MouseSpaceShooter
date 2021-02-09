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

		ProgramBase(ProgramId program) :
			program(program)
		{}

		bool isValid() const
		{
			return program != 0;
		}

		operator ProgramId()
		{
			return program;
		}

		ProgramId program;
	};
}
