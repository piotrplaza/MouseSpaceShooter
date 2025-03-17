#pragma once

#include <ogl/shadersUtils.hpp>
#include <ogl/uniformsUtils.hpp>

#include <functional>

namespace ShadersUtils
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

		virtual ~ProgramBase() = default;

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

	template <typename Derived>
	class ProgramBaseCRTP : public ProgramBase
	{
	public:
		ProgramBaseCRTP() = default;

		ProgramBaseCRTP(ProgramId program) :
			ProgramBase(program)
		{}

		std::function<void(Derived&)> frameSetupF;
	};
}
