#pragma once

#include "programBase.hpp"

namespace ShadersUtils
{
	namespace Programs
	{
		struct TrailsAccessor : AccessorBase
		{
			using AccessorBase::AccessorBase;

			TrailsAccessor(ProgramId program):
				AccessorBase(program),
				vp(program, "vp"),
				color(program, "color"),
				deltaTime(program, "deltaTime", false),
				deltaTimeFactor(program, "deltaTimeFactor")
			{
			}

			UniformsUtils::UniformMat4f vp;
			UniformsUtils::Uniform4f color;
			UniformsUtils::Uniform1f deltaTime;
			UniformsUtils::Uniform1f deltaTimeFactor;
		};

		struct Trails : ProgramBase<TrailsAccessor>
		{
			Trails() :
				ProgramBase(LinkProgram(CompileShaders("ogl/shaders/trails.vs",
					"ogl/shaders/trails.gs", "ogl/shaders/trails.fs"), { {0, "bPos"}, {1, "bColor"}, {2, "bVelocityAndTime"} }))
			{
				vp(glm::mat4(1.0f));
				color(glm::vec4(1.0f));
				deltaTime(0.0f);
				deltaTimeFactor(1.0f);
			}

			Trails(const Trails&) = delete;

			~Trails()
			{
				glDeleteProgram(getProgramId());
			}
		};
	}
}
