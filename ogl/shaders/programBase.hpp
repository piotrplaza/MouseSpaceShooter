#pragma once

#include <ogl/shaders.hpp>
#include <ogl/uniforms.hpp>

namespace Shaders
{
	class ProgramBase
	{
	public:
		ProgramBase() :
			program(0)
		{}

		ProgramBase(ProgramId program) :
			program(program)
		{}

		operator ProgramId() const
		{
			return program;
		}

		ProgramId getProgramId() const
		{
			return program;
		}

		bool isValid() const
		{
			return program != 0;
		}

	private:
		ProgramId program;
	};
}
