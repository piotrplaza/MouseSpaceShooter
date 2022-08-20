#pragma once

#include "../level.hpp"

#include <memory>

namespace Levels
{
	class Dzidzia2: public Level
	{
	public:
		Dzidzia2();
		~Dzidzia2();

		void step() override;

	private:
		class Impl;
		std::unique_ptr<Impl> impl;
	};
}
