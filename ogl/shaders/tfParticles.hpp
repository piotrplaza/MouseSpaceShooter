#pragma once

#include "programBase.hpp"

namespace ShadersUtils
{
	namespace Programs
	{
		struct TFParticlesAccessor : AccessorBase
		{
			using AccessorBase::AccessorBase;

			TFParticlesAccessor(ProgramId program):
				AccessorBase(program),
				componentId(program, "componentId"),
				time(program, "time", false),
				deltaTime(program, "deltaTime", false),
				particlesCount(program, "particlesCount"),
				lifeTimeRange(program, "lifeTimeRange"),
				init(program, "init"),
				respawning(program, "respawning"),
				originInit(program, "originInit"),
				originBegin(program, "originBegin"),
				originEnd(program, "originEnd"),
				originForce(program, "originForce"),
				velocityOffset(program, "velocityOffset"),
				initVelocity(program, "initVelocity"),
				velocitySpreadFactorRange(program, "velocitySpreadFactorRange"),
				velocityRotateZHRange(program, "velocityRotateZHRange"),
				velocityFactor(program, "velocityFactor"),
				colorRange(program, "colorRange"),
				globalForce(program, "globalForce"),
				AZPlusBPlusCT(program, "AZPlusBPlusCT")
			{
			}

			UniformsUtils::Uniform1i componentId;
			UniformsUtils::Uniform1f time;
			UniformsUtils::Uniform1f deltaTime;
			UniformsUtils::Uniform1i particlesCount;
			UniformsUtils::Uniform2f lifeTimeRange;
			UniformsUtils::Uniform1b init;
			UniformsUtils::Uniform1b respawning;
			UniformsUtils::Uniform1b originInit;
			UniformsUtils::Uniform3f originBegin;
			UniformsUtils::Uniform3f originEnd;
			UniformsUtils::Uniform1f originForce;
			UniformsUtils::Uniform3f velocityOffset;
			UniformsUtils::Uniform3f initVelocity;
			UniformsUtils::Uniform2f velocitySpreadFactorRange;
			UniformsUtils::Uniform1f velocityRotateZHRange;
			UniformsUtils::Uniform1f velocityFactor;
			UniformsUtils::Uniform4fv<2> colorRange;
			UniformsUtils::Uniform3f globalForce;
			UniformsUtils::Uniform3f AZPlusBPlusCT;
		};

		struct TFParticles : ProgramBase<TFParticlesAccessor>
		{
			TFParticles():
				ProgramBase(LinkProgram(CompileVertexShader("ogl/shaders/tfParticles.vs"), { {0, "bPos"}, {1, "bColor"}, {2, "bVelocityAndTime"}, {3, "bHSizeAndAngleAttribIdx"} }, [](ProgramId program) {
					const std::array tfOutput = { "vPos", "vColor", "vVelocityAndTime", "vHSizeAndAngleAttribIdx" };
					glTransformFeedbackVaryings(program, (unsigned)tfOutput.size(), tfOutput.data(), GL_SEPARATE_ATTRIBS);
				}))
			{
				componentId(1);
				time(1.0f);
				deltaTime(0.0f);
				lifeTimeRange(glm::vec2(1.0f));
				particlesCount(0);
				init(false);
				respawning(true);
				originInit(false);
				originBegin(glm::vec3(0.0f));
				originEnd(glm::vec3(0.0f));
				originForce(0.0f);
				velocityOffset(glm::vec3(0.0f));
				initVelocity(glm::vec3(0.0f));
				velocitySpreadFactorRange(glm::vec2(1.0f));
				velocityRotateZHRange(0.0f);
				velocityFactor(1.0f);
				colorRange(glm::vec4(1.0f));
				globalForce(glm::vec3(0.0f));
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
