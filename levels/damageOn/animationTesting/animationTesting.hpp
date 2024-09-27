#pragma once

#include "../../level.hpp"

#include <memory>

namespace Levels::DamageOn
{
	class AnimationTesting: public Level
	{
	public:
		AnimationTesting();
		~AnimationTesting();

		void postSetup() override;
		void step() override;

	private:
		class Impl;
		std::unique_ptr<Impl> impl;
	};
}
