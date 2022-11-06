#pragma once

#include "../level.hpp"

#include <memory>

namespace Levels
{
	class RaceBase: public Level
	{
	public:
		RaceBase();
		~RaceBase();

		void step() override;

	private:
		class Impl;
		std::unique_ptr<Impl> impl;
	};
}
