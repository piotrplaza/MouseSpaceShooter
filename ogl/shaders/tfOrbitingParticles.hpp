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
				ProgramBase(program),
				deltaTime(program, "deltaTime")
			{
			}

			UniformsUtils::Uniform1f deltaTime;
		};

		struct TFOrbitingParticles : TFOrbitingParticlesAccessor
		{
			TFOrbitingParticles():
				TFOrbitingParticlesAccessor(LinkProgram(CompileVertexShader("ogl/shaders/tfOrbitingParticles.vs"), { {0, "bPos"}, {1, "bColor"}, {2, "bVelocityAndTime"}, {3, "bHSizeAndAngleAttribIdx"} }, [](ProgramId program) {
					const std::array tfOutput = { "vPos", "vColor", "vVelocityAndTime", "vHSizeAndAngleAttribIdx" };
					glTransformFeedbackVaryings(program, tfOutput.size(), tfOutput.data(), GL_SEPARATE_ATTRIBS);
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
