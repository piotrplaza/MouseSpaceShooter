#pragma once

#include "../../level.hpp"

#include <memory>

namespace Levels::DamageOn
{
	class Nest: public Level
	{
	public:
		Nest();
		~Nest();

		void postSetup() override;
		void step() override;

	private:
		class Impl;
		std::unique_ptr<Impl> impl;
	};
}
