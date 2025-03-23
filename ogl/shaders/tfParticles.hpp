#pragma once

#include "programBase.hpp"

namespace ShadersUtils
{
	namespace Programs
	{
		struct TFParticlesAccessor : ProgramBaseCRTP<TFParticlesAccessor>
		{
			using ProgramBaseCRTP::ProgramBaseCRTP;

			TFParticlesAccessor(ProgramId program):
				ProgramBaseCRTP(program),
				deltaTime(program, "deltaTime"),
				velocitySpreadRange(program, "velocitySpreadRange")
			{
			}

			UniformsUtils::Uniform1f deltaTime;
			UniformsUtils::Uniform2f velocitySpreadRange;
			UniformsUtils::Uniform1b mode3D;
		};

		struct TFParticles : TFParticlesAccessor
		{
			TFParticles():
				TFParticlesAccessor(LinkProgram(CompileVertexShader("ogl/shaders/tfParticles.vs"), { {0, "bPos"}, {1, "bColor"}, {2, "bVelocityAndTime"}, {3, "bHSizeAndAngleAttribIdx"} }, [](ProgramId program) {
					const std::array tfOutput = { "vPos", "vColor", "vVelocityAndTime", "vHSizeAndAngleAttribIdx" };
					glTransformFeedbackVaryings(program, (unsigned)tfOutput.size(), tfOutput.data(), GL_SEPARATE_ATTRIBS);
				}))
			{
				deltaTime(0.0f);
				velocitySpreadRange(glm::vec2(0.0f));
			}

			TFParticles(const TFParticles&) = delete;

			~TFParticles()
			{
				glDeleteProgram(getProgramId());
			}
		};
	}
}
