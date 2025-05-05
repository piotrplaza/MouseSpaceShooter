#pragma once

#include "programBase.hpp"

namespace ShadersUtils
{
	namespace Programs
	{
		struct BillboardsAccessor : AccessorBase
		{
			using AccessorBase::AccessorBase;

			BillboardsAccessor(ProgramId program):
				AccessorBase(program),
				vp(program, "vp"),
				color(program, "color"),
				texture0(program, "texture0")
			{
			}

			UniformsUtils::UniformMat4f vp;
			UniformsUtils::Uniform4f color;
			UniformsUtils::Uniform1i texture0;
		};

		struct Billboards : ProgramBase<BillboardsAccessor>
		{
			Billboards() :
				ProgramBase(LinkProgram(CompileShaders("ogl/shaders/billboards.vs",
					"ogl/shaders/billboards.gs", "ogl/shaders/billboards.fs"), { {0, "bPos"}, {1, "bColor"} }))
			{
				vp(glm::mat4(1.0f));
				color(glm::vec4(1.0f));
			}

			Billboards(const Billboards&) = delete;

			~Billboards()
			{
				glDeleteProgram(getProgramId());
			}
		};
	}
}
