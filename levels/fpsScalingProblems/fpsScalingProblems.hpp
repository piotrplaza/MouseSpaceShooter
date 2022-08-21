#pragma once

#include "../level.hpp"

#include <memory>

namespace Levels
{
	class FPSScalingProblems: public Level
	{
	public:
		FPSScalingProblems();
		~FPSScalingProblems();

		void step() override;

	private:
		class Impl;
		std::unique_ptr<Impl> impl;
	};
}
