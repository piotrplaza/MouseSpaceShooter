#pragma once

#include "../level.hpp"

#include <memory>

namespace Levels
{
	class SquareRace: public Level
	{
	public:
		SquareRace();
		~SquareRace();

		void step() override;

	private:
		class Impl;
		std::unique_ptr<Impl> impl;
	};
}
