#pragma once

#include "../level.hpp"

#include <memory>

namespace Levels
{
	class Dzidzia: public Level
	{
	public:
		Dzidzia();
		~Dzidzia();

		void step() override;

	private:
		class Impl;
		std::unique_ptr<Impl> impl;
	};
}
