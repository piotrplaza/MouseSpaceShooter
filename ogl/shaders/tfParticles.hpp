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
				time(program, "time"),
				deltaTime(program, "deltaTime"),
				particlesCount(program, "particlesCount"),
				lifeTimeRange(program, "lifeTimeRange"),
				init(program, "init"),
				respawning(program, "respawning"),
				originBegin(program, "originBegin"),
				originEnd(program, "originEnd"),
				initVelocity(program, "initVelocity"),
				velocitySpreadFactorRange(program, "velocitySpreadFactorRange"),
				velocityRotateZHRange(program, "velocityRotateZHRange"),
				velocityFactor(program, "velocityFactor"),
				colorRange(program, "colorRange"),
				gravity(program, "gravity"),
				AZPlusBPlusCT(program, "AZPlusBPlusCT")
			{
			}

			UniformsUtils::Uniform1f time;
			UniformsUtils::Uniform1f deltaTime;
			UniformsUtils::Uniform1i particlesCount;
			UniformsUtils::Uniform2f lifeTimeRange;
			UniformsUtils::Uniform1b init;
			UniformsUtils::Uniform1b respawning;
			UniformsUtils::Uniform3f originBegin;
			UniformsUtils::Uniform3f originEnd;
			UniformsUtils::Uniform3f initVelocity;
			UniformsUtils::Uniform2f velocitySpreadFactorRange;
			UniformsUtils::Uniform1f velocityRotateZHRange;
			UniformsUtils::Uniform1f velocityFactor;
			UniformsUtils::Uniform4fv<2> colorRange;
			UniformsUtils::Uniform3f gravity;
			UniformsUtils::Uniform3f AZPlusBPlusCT;
		};

		struct TFParticles : TFParticlesAccessor
		{
			TFParticles():
				TFParticlesAccessor(LinkProgram(CompileVertexShader("ogl/shaders/tfParticles.vs"), { {0, "bPos"}, {1, "bColor"}, {2, "bVelocityAndTime"}, {3, "bHSizeAndAngleAttribIdx"} }, [](ProgramId program) {
					const std::array tfOutput = { "vPos", "vColor", "vVelocityAndTime", "vHSizeAndAngleAttribIdx" };
					glTransformFeedbackVaryings(program, (unsigned)tfOutput.size(), tfOutput.data(), GL_SEPARATE_ATTRIBS);
				}))
			{
				time(1.0f);
				deltaTime(0.0f);
				lifeTimeRange(glm::vec2(1.0f));
				particlesCount(0);
				init(false);
				respawning(true);
				originBegin(glm::vec3(0.0f));
				originEnd(glm::vec3(0.0f));
				initVelocity(glm::vec3(0.0f));
				velocitySpreadFactorRange(glm::vec2(1.0f));
				velocityRotateZHRange(0.0f);
				velocityFactor(1.0f);
				colorRange(glm::vec4(1.0f));
				gravity(glm::vec3(0.0f));
				AZPlusBPlusCT(glm::vec3(1.0f, 0.0f, 0.0f));
			}

			TFParticles(const TFParticles&) = delete;

			~TFParticles()
			{
				glDeleteProgram(getProgramId());
			}
		};
	}
}
