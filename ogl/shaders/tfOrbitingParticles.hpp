#pragma once

#include "programBase.hpp"

namespace ShadersUtils
{
	namespace Programs
	{
		struct TFOrbitingParticlesAccessor : ProgramBase<TFOrbitingParticlesAccessor>
		{
			using ProgramBase::ProgramBase;

			TFOrbitingParticlesAccessor(ProgramId program):
				ProgramBase(program)
			{
			}
		};

		struct TFOrbitingParticles : TFOrbitingParticlesAccessor
		{
			TFOrbitingParticles():
				TFOrbitingParticlesAccessor(LinkProgram(CompileVertexShader("ogl/shaders/tfOrbitingParticles.vs"), { {0, "bPos"}, {1, "bColor"} }, [](ProgramId program) {
					const GLchar* tfOutput[] = { "vPos", "vColor" };
					glTransformFeedbackVaryings(program, 2, tfOutput, GL_SEPARATE_ATTRIBS);
				}))
			{
			}

			TFOrbitingParticles(const TFOrbitingParticles&) = delete;

			~TFOrbitingParticles()
			{
				glDeleteProgram(getProgramId());
			}
		};
	}
}
