#pragma once

#include "programBase.hpp"

namespace ShadersUtils
{
	namespace Programs
	{
		struct BasicAccessor : AccessorBase
		{
			using AccessorBase::AccessorBase;

			BasicAccessor(ProgramId program):
				AccessorBase(program),
				model(program, "model"),
				vp(program, "vp"),
				color(program, "color"),
				forcedAlpha(program, "forcedAlpha")
			{
			}

			UniformsUtils::UniformMat4f model;
			UniformsUtils::UniformMat4f vp;
			UniformsUtils::Uniform4f color;
			UniformsUtils::Uniform1f forcedAlpha;
		};

		struct Basic : ProgramBase<BasicAccessor>
		{
			Basic():
				ProgramBase(LinkProgram(CompileShaders("ogl/shaders/basic.vs",
					"ogl/shaders/basic.fs"), { {0, "bPos"}, {1, "bColor"}, {4, "bInstancedTransform"} }))
			{
				model(glm::mat4(1.0f));
				vp(glm::mat4(1.0f));
				color(glm::vec4(1.0f));
				forcedAlpha(-1.0f);
			}

			Basic(const Basic&) = delete;

			~Basic()
			{
				glDeleteProgram(getProgramId());
			}
		};
	}
}
