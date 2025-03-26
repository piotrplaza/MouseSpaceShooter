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
				lifeTimeRange(program, "lifeTimeRange"),
				restart(program, "restart"),
				respawning(program, "respawning"),
				origin(program, "origin"),
				initVelocity(program, "initVelocity"),
				velocitySpreadFactorRange(program, "velocitySpreadFactorRange"),
				velocityRotateZHRange(program, "velocityRotateZHRange"),
				colorRange(program, "colorRange"),
				gravity(program, "gravity")
			{
			}

			UniformsUtils::Uniform1f deltaTime;
			UniformsUtils::Uniform2f lifeTimeRange;
			UniformsUtils::Uniform1b restart;
			UniformsUtils::Uniform1b respawning;
			UniformsUtils::Uniform3f origin;
			UniformsUtils::Uniform3f initVelocity;
			UniformsUtils::Uniform2f velocitySpreadFactorRange;
			UniformsUtils::Uniform1f velocityRotateZHRange;
			UniformsUtils::Uniform4fv<2> colorRange;
			UniformsUtils::Uniform3f gravity;
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
				lifeTimeRange(glm::vec2(1.0f));
				restart(false);
				respawning(true);
				origin(glm::vec3(0.0f));
				initVelocity(glm::vec3(0.0f));
				velocitySpreadFactorRange(glm::vec2(1.0f));
				velocityRotateZHRange(0.0f);
				colorRange(glm::vec4(1.0f));
				gravity(glm::vec3(0.0f));
			}

			TFParticles(const TFParticles&) = delete;

			~TFParticles()
			{
				glDeleteProgram(getProgramId());
			}
		};
	}
}
