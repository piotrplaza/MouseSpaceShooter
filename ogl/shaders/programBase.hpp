#pragma once

#include <ogl/shadersUtils.hpp>
#include <ogl/uniformsUtils.hpp>

#include <memory>
#include <functional>
#include <vector>
#include <unordered_map>
#include <unordered_set>

namespace ShadersUtils
{
	struct AccessorBase
	{
		friend class UniformsUtils::Uniform;

		AccessorBase() :
			program(0)
		{
		}

		AccessorBase(ProgramId program) :
			program(program)
		{
			if (program != 0)
				programIdsToPrograms.insert({ program, this });
		}

		virtual ~AccessorBase() = default;

		operator ProgramId() const
		{
			return program;
		}

		ProgramId getProgramId() const
		{
			return program;
		}

		bool isValid() const
		{
			return program != 0;
		}

		void saveUniforms()
		{
			assert(uniformCheckpoints.empty());
			uniformCheckpoints.reserve(uniforms.size());
			for (auto [id, uniform] : uniforms)
				uniformCheckpoints.push_back(uniform->getCheckpoint());
		}

		void restoreUniforms()
		{
			assert(!uniformCheckpoints.empty());
			for (auto& uniformCheckpoint : uniformCheckpoints)
				uniformCheckpoint();
			uniformCheckpoints.clear();
		}

		std::vector<std::function<void()>> getUniformsCheckpoint() const
		{
			std::vector<std::function<void()>> uniformCheckpoints;
			uniformCheckpoints.reserve(uniforms.size());
			for (auto [id, uniform] : uniforms)
				uniformCheckpoints.push_back(uniform->getCheckpoint());
			return uniformCheckpoints;
		}

		void restoreUniformsCheckpoint(const std::vector<std::function<void()>>& uniformsCheckpoint)
		{
			assert(!uniformsCheckpoint.empty());
			for (auto& uniformCheckpoint : uniformsCheckpoint)
				uniformCheckpoint();
		}

	protected:
		static inline std::unordered_map<ProgramId, AccessorBase*> programIdsToPrograms;

	private:
		ProgramId program;
		std::unordered_map<GLint, UniformsUtils::Uniform*> uniforms;
		std::vector<std::function<void()>> uniformCheckpoints;
	};

	class ProgramFrameSetup
	{
	public:
		static void FrameSetup()
		{
			for (auto program : activePrograms)
				program->frameSetup();
		}

		virtual void frameSetup() = 0;

	protected:
		inline static std::unordered_set<ProgramFrameSetup*> activePrograms;
	};

	template <typename Accessor>
	struct ProgramBase : ProgramFrameSetup, Accessor
	{
		ProgramBase(ProgramId program) :
			Accessor(program)
		{
			activePrograms.insert(this);
		}

		~ProgramBase() override
		{
			assert(this->getProgramId() != 0);
			AccessorBase::programIdsToPrograms.erase(this->getProgramId());
			activePrograms.erase(this);
		}

		void frameSetup() override
		{
			if (frameSetupF)
				frameSetupF(*this);
		}

		std::function<void(Accessor&)> frameSetupF;
	};
}
