#pragma once

#include "programBase.hpp"

namespace ShadersUtils
{
	namespace Programs
	{
		struct EffectsAccessor : AccessorBase
		{
			using AccessorBase::AccessorBase;

			EffectsAccessor(ProgramId program):
				AccessorBase(program),
				//texture(program, "texture")
				time(program, "time"),
				deltaTime(program, "deltaTime")
			{
			}

			//UniformsUtils::Uniform1i texture;
			UniformsUtils::Uniform1f time;
			UniformsUtils::Uniform1f deltaTime;
		};

		struct Effects : ProgramBase<EffectsAccessor>
		{
			Effects():
				ProgramBase(LinkProgram(CompileShaders("ogl/shaders/effects.vs",
					"ogl/shaders/effects.fs"), { {0, "bPos"}, {2, "bTexCoord"} }))
			{
				time(0.0f);
				deltaTime(0.0f);
			}

			Effects(const Effects&) = delete;

			~Effects()
			{
				glDeleteProgram(getProgramId());
			}
		};
	}
}
